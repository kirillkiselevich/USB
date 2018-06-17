#include <iostream>
#include <libusb.h>
using namespace std;

void printdev(libusb_device *dev); //prototype of the function

int main() {
	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	libusb_device_handle *dev_handle; //a device handle
	libusb_context *ctx = NULL; //a libusb session
	int r; //for return values
	ssize_t cnt; //holding number of devices in list
	int vendorID,productID;
	r = libusb_init(&ctx); //initialize a library session
	if(r < 0) {
		cout<<"Ошибка инициализации"<<r<<endl; //there was an error
		return 1;
	}
	libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
	if(cnt < 0) {
		cout<<"Ошибка получения устройств"<<endl; //there was an error
		return 1;
	}
	cout<<cnt<<" Список устройств."<<endl; //print total number of usb devices
	ssize_t i; //for iterating through the list
	for(i = 0; i < cnt; i++) {
		printdev(devs[i]); //print specs of this device
	}

	//libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	cout << "VendorID: ";
	cin >> vendorID;
	cout << "\n";
	cout << "ProductID: ";
	cin >> productID;
	dev_handle = libusb_open_device_with_vid_pid(ctx, vendorID, productID); //these are vendorID and productID I found for my usb device
	if(dev_handle == NULL)
		cout<<"Невозможно поключиться к устройству"<<endl;
	else
		cout<<"Устройство подключенно"<<endl;
	libusb_free_device_list(devs, 1); //free the list, unref the devices in it

	unsigned char *data = new unsigned char[4]; //data to write
	data[0]='a';data[1]='b';data[2]='c';data[3]='d'; //some dummy values

	int actual; //used to find out how many bytes were written
	if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
		cout<<"Драйвер ядра активен"<<endl;
		if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
			cout<<"Отсоединен драйвер ядра!"<<endl;
	}
	r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
	if(r < 0) {
		cout<<"Не удается запросить интерфейс"<<endl;
		return 1;
	}
	cout<<"Интерфейс запрошен"<<endl;
	
	cout<<"Данные->"<<data<<"<-"<<endl; //just to see the data we want to write : abcd
	cout<<"Запись данных..."<<endl;
	r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT), data, 4, &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129
	if(r == 0 && actual == 4) //we wrote the 4 bytes successfully
		cout<<"Запись прошла успешно!"<<endl;
	else
		cout<<"Ошибка записи"<<endl;
	
	r = libusb_release_interface(dev_handle, 0); //release the claimed interface
	if(r!=0) {
		cout<<"Не удается освободить интерфейс"<<endl;
		return 1;
	}
	cout<<"Интерфейс освобожден"<<endl;

	libusb_close(dev_handle); //close the device we opened
	libusb_exit(ctx); //needs to be called to end the

	delete[] data; //delete the allocated memory for data

	return 0;
}

void printdev(libusb_device *dev) {
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		cout<<"Ну удалось получить дискриптор устройства"<<endl;
		return;
	}
	cout<<"Список возможных конфигураций: "<<(int)desc.bNumConfigurations<<"  \n";
	cout<<"Класс устройства: "<<(int)desc.bDeviceClass<<"  \n";
	cout<<"VendorID: (hex) " <<hex<<desc.idVendor<<dec<<" (dec) "<<desc.idVendor<<"  \n";
	cout<<"ProductID: (hex) "<<hex<<desc.idProduct<<dec<<" (dec) "<<desc.idProduct<<"  \n"<<endl;
}
