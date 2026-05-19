#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/err.h>

static unsigned int bus_num = 0;
static unsigned short i2c_addr = 0x31;
static unsigned int reg1 = 0x10;
static unsigned int reg2 = 0x11;
static unsigned int val = 0x0;

module_param(bus_num, uint, 0444);
MODULE_PARM_DESC(bus_num, "I2C bus number");

module_param(i2c_addr, ushort, 0444);
MODULE_PARM_DESC(i2c_addr, "7-bit I2C device address");

module_param(reg1, uint, 0444);
MODULE_PARM_DESC(reg1, "Register address1");

module_param(reg2, uint, 0444);
MODULE_PARM_DESC(reg2, "Register address1");

module_param(val, uint, 0444);
MODULE_PARM_DESC(val, "Register value");

static struct i2c_client *vep_i2c_client;

static int __init vep_i2c_write_init(void)
{
    struct i2c_adapter *adap;
    int ret;

    pr_info("vep-dsa: Turning off tx-disable via setting i2c registers...\n");

    adap = i2c_get_adapter(bus_num);
    if (!adap) {
        pr_err("vep-dsa: failed to get i2c adapter %u\n", bus_num);
        return -ENODEV;
    }

    if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE_DATA)) {
        pr_err("vep-dsa: adapter does not support SMBus byte data\n");
        i2c_put_adapter(adap);
        return -EOPNOTSUPP;
    }

    vep_i2c_client = i2c_new_dummy_device(adap, i2c_addr);
    i2c_put_adapter(adap);

    if (IS_ERR(vep_i2c_client)) {
        ret = PTR_ERR(vep_i2c_client);
        pr_err("vep-dsa: failed to create i2c client: %d\n", ret);
        return ret;
    }

    if (i2c_smbus_write_byte_data(vep_i2c_client, 0x10, 0x0) < 0 || \
        i2c_smbus_write_byte_data(vep_i2c_client, 0x11, 0x0) < 0) {
        pr_err("vep-dsa: i2c write failed\n");
        i2c_unregister_device(vep_i2c_client);
        vep_i2c_client = NULL;
        return -EIO;
    }

    pr_info("vep-dsa: SFP TX power enabled.\n");
    i2c_unregister_device(vep_i2c_client);
    vep_i2c_client = NULL;
    return 0;
}

static void __exit vep_i2c_write_exit(void)
{
    if (vep_i2c_client) {
        i2c_unregister_device(vep_i2c_client);
        vep_i2c_client = NULL;
    }

    pr_info("vep-dsa: unloaded\n");
}

module_init(vep_i2c_write_init);
module_exit(vep_i2c_write_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hansel Tsao");
MODULE_DESCRIPTION("VEP1400 DSA I2C write module to enable SFP TX power");