#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input-polldev.h>

MODULE_LICENSE("GPL");

struct nunchuk_dev {
	struct i2c_client *i2c_client;
};

static char* buf1;
static char* buf2;
static char* buff;
static int buf_size = 2;
static int zpressed = 1;
static int cpressed = 1;

int nunchuk_read_registers(struct i2c_client *client, char* buff)
{
	char buf_init[1] = {0x0};

	mdelay(10);

	if(i2c_master_send(client, buf_init, 1) <= 0)
	{
		return -1;
	}

	mdelay(10);

	if(i2c_master_recv(client, buff, 6) <= 0)
	{
		return -1;
	}

	return 0;
}

void nunchuk_poll (struct input_polled_dev *polled_input){
	
	int i;
	int zpressed_new;
	int cpressed_new;
	struct nunchuk_dev* nunchuk;
	
	nunchuk = polled_input->private;
	
	if(nunchuk_read_registers(nunchuk->i2c_client, buff) == -1)
	{
		pr_info("Something is wrong in function nunchuk_read_registers\n");
	}
	
	/*
	for (i = 0; i < 6; i++)
	{
		pr_info("Second buff[%d]: 0x%x\n", i, buff[i]);
	}
	*/
	
	zpressed_new = (buff[5] & 0x1);
	cpressed_new = (buff[5] & 0x2);
	
	if ((zpressed_new == 0) && (zpressed != zpressed_new))
	{
		pr_info("Z is pressed!\n");
	}
	else if (zpressed != zpressed_new)
	{
		pr_info("Z is not pressed!\n");
	}
	
	if ((cpressed_new == 0) && (cpressed != cpressed_new))
	{
		pr_info("C is pressed!\n");
	}
	else if (cpressed != cpressed_new)
	{
		pr_info("C is not pressed!\n");
	}
	
	zpressed = zpressed_new;
	cpressed = cpressed_new;
	
	//input_event(polled_input->input, EV_KEY, BTN_Z, zpressed);
	//input_event(polled_input->input, EV_KEY, BTN_C, cpressed);
	//input_sync(polled_input->input);
}


static int nunchuk_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{	
	int vred;
	//int i;

	struct nunchuk_dev *nunchuk;
	struct input_polled_dev* polled_input;
	struct input_dev* input;
	
	nunchuk = devm_kzalloc(&client->dev, sizeof(struct nunchuk_dev), GFP_KERNEL);
	
	if (!nunchuk) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}
	
	polled_input = devm_input_allocate_polled_device(&client->dev);
	input = polled_input->input;

	nunchuk->i2c_client = client;
	polled_input->private = nunchuk;

	input->name = "Wii Nunchuk";
	input->id.bustype = BUS_I2C;
	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_C, input->keybit);
	set_bit(BTN_Z, input->keybit);

	polled_input->poll = nunchuk_poll; //OVDE
	polled_input->poll_interval = 50;

	input_register_polled_device(polled_input);
	
	pr_info("Probe function!\n");
	
	buff = kzalloc(6, GFP_KERNEL);
	buf1 = kzalloc(buf_size, GFP_KERNEL);
	buf2 = kzalloc(buf_size, GFP_KERNEL);
	
	buf1[0] = 0xf0;
	buf1[1] = 0x55;
	
	buf2[0] = 0xfb;
	buf2[1] = 0x0;
	
	vred = i2c_master_send(client, buf1, buf_size);
	
	pr_info("Buf1: %d\n", vred);
	
	udelay(1);
	
	vred = i2c_master_send(client, buf2, buf_size);
	
	pr_info("Buf2: %d\n", vred);
/*
	if(nunchuk_read_registers(client, buff) == -1)
	{
		pr_info("Something is wrong in function nunchuk_read_registers\n");
		return 0;
	}
	
	for (i = 0; i < 6; i++)
	{
		pr_info("First buff[%d]: 0x%x\n", i, buff[i]);
	}
	
	if(nunchuk_read_registers(client, buff) == -1)
	{
		pr_info("Something is wrong in function nunchuk_read_registers\n");
		return 0;
	}
	
	for (i = 0; i < 6; i++)
	{
		pr_info("Second buff[%d]: 0x%x\n", i, buff[i]);
	}
	
	zpressed = (buff[5] & 0x1);
	cpressed = (buff[5] & 0x2);
	
	if(zpressed == 0)
	{
		pr_info("Z is pressed!\n");
	}
	else
	{
		pr_info("Z is not pressed!\n");
	}
	
	if(cpressed == 0)
	{
		pr_info("C is pressed!\n");
	}
	else
	{
		pr_info("C is not pressed!\n");
	}
*/
	return 0;
}

static int nunchuk_remove(struct i2c_client *client)
{
	pr_info("REMOVE!\n");
	return 0;
}

static const struct i2c_device_id nunchuk_id[] = {
	{ "nunchuk", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, nunchuk_id);

#ifdef CONFIG_OF
static struct of_device_id nunchuk_dt_match[] = {
	{ .compatible = "nintendo" },
	{ },
};
#endif

static struct i2c_driver nunchuk_driver = {
	.driver = {
		.name = "nunchuk",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nunchuk_dt_match),
	},
	.probe = nunchuk_probe,
	.remove = nunchuk_remove,
	.id_table = nunchuk_id,
};

module_i2c_driver(nunchuk_driver);

