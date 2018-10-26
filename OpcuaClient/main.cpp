#include <stdio.h>
#include "open62541.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <windows.h>
#include <thread>
using namespace cv;
using namespace std;

typedef struct {
	UA_Byte x[61542];
} framepixel;
UA_Boolean running = true;
UA_Logger logger = UA_Log_Stdout;
UA_Int32 gonum = 680;
void show()
{
	UA_Client *client = UA_Client_new(UA_ClientConfig_default);
	UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840");
	//UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://192.168.0.7:4840");
	if (retval != UA_STATUSCODE_GOOD) {
		UA_Client_delete(client);
		return;
		Sleep(1000);
	}
	while (running) {
		//////////////////////////////
		UA_Variant value; /* Variants can hold scalar values and arrays of any type */
		UA_Variant_init(&value);
		UA_NodeId nodeId = UA_NODEID_STRING(1, "the.answer");
		UA_Client_readValueAttribute(client, nodeId, &value);

		UA_Int32 v = *(UA_Int32 *)value.data;
		cout << v << endl;


		///////////////////////////////

		UA_Variant_init(&value);
		nodeId = UA_NODEID_STRING(1, "frame Variable");
		UA_Client_readValueAttribute(client, nodeId, &value);

		framepixel *p = (framepixel *)value.data;

		Mat Main_frame;
		Main_frame.create(480, 640, CV_8UC3);
		Main_frame.data = p->x;


		namedWindow("Display window", WINDOW_AUTOSIZE);
		imshow("Display window", Main_frame);

		waitKey(1);
		//////////////////////////write 
		UA_Variant_init(&value);
		gonum++;
		UA_Variant_setScalarCopy(&value, &gonum, &UA_TYPES[UA_TYPES_INT32]);
		UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "go"), &value);
		////////////////////////////////
	}
	UA_Client_disconnect(client);
	UA_Client_delete(client);
}
static void stopHandler(int sign) {
	UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "Received Ctrl-C");
	running = 0;
}



int main(int argc, char *argv[]) {



	thread t(show);
	system("pause");
	running = 0;
	t.join();

	return 0;
}