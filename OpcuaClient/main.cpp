#include <stdio.h>
#include "open62541.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <windows.h>
using namespace cv;
using namespace std;

typedef struct {
	UA_Byte x[61542];
} framepixel;


int main(int argc, char *argv[]) {
	UA_Client *client = UA_Client_new(UA_ClientConfig_default);
	UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); 
	//UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://192.168.0.7:4840");
	if (retval != UA_STATUSCODE_GOOD) {
		UA_Client_delete(client);
		return (int)retval;
	}
	//////////////////////////////
	UA_Variant value; /* Variants can hold scalar values and arrays of any type */
	UA_Variant_init(&value);
	UA_NodeId nodeId =UA_NODEID_STRING(1, "the.answer");
	retval = UA_Client_readValueAttribute(client, nodeId, &value);
	if (retval == UA_STATUSCODE_GOOD) {
		UA_Int32 v = *(UA_Int32 *)value.data;
		cout << v<<endl;
	}
	
	///////////////////////////////

	UA_Variant_init(&value);
	nodeId = UA_NODEID_STRING(1, "frame Variable");
	retval = UA_Client_readValueAttribute(client, nodeId, &value);
	if (retval == UA_STATUSCODE_GOOD) {
		 
		framepixel *p=(framepixel *)value.data;
		
		Mat Main_frame;
	    Main_frame.create(234, 263, CV_8UC1);
		Main_frame.data = p->x;
		

		namedWindow("Display window", WINDOW_AUTOSIZE);
		imshow("Display window", Main_frame);

		waitKey(1);

	}

	
	system("pause");
	UA_Client_disconnect(client);
	UA_Client_delete(client);
	return (int)UA_STATUSCODE_GOOD;
}