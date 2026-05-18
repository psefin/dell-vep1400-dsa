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

    pr_info("vep-i2c-write: bus=%u addr=0x%02x reg1=0x%02x reg2=0x%02x val=0x%02x\n",
            bus_num, i2c_addr, reg1, reg2, val);

    adap = i2c_get_adapter(bus_num);
    if (!adap) {
        pr_err("vep-i2c-write: failed to get i2c adapter %u\n", bus_num);
        return -ENODEV;
    }

    if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE_DATA)) {
        pr_err("vep-i2c-write: adapter does not support SMBus byte data\n");
        i2c_put_adapter(adap);
        return -EOPNOTSUPP;
    }

    vep_i2c_client = i2c_new_dummy_device(adap, i2c_addr);
    i2c_put_adapter(adap);

    if (IS_ERR(vep_i2c_client)) {
        ret = PTR_ERR(vep_i2c_client);
        pr_err("vep-i2c-write: failed to create i2c client: %d\n", ret);
        return ret;
    }

    ret = i2c_smbus_write_byte_data(vep_i2c_client, reg1 & 0xff, val & 0xff);
    if (ret < 0) {
        pr_err("vep-i2c-write: i2c write reg1 failed: %d\n", ret);
        i2c_unregister_device(vep_i2c_client);
        vep_i2c_client = NULL;
        return ret;
    }
    ret = i2c_smbus_write_byte_data(vep_i2c_client, reg2 & 0xff, val & 0xff);
    if (ret < 0) {
        pr_err("vep-i2c-write: i2c write reg2 failed: %d\n", ret);
        i2c_unregister_device(vep_i2c_client);
        vep_i2c_client = NULL;
        return ret;
    }

    pr_info("vep-i2c-write: write ok\n");
    return 0;
}

static void __exit vep_i2c_write_exit(void)
{
    if (vep_i2c_client) {
        i2c_unregister_device(vep_i2c_client);
        vep_i2c_client = NULL;
    }

    pr_info("vep-i2c-write: unloaded\n");
}

module_init(vep_i2c_write_init);
module_exit(vep_i2c_write_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hansel Tsao");
MODULE_DESCRIPTION("Example kernel module equivalent to i2cset");