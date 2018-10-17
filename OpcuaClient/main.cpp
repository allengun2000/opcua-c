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
UA_Boolean running = true;
UA_Logger logger = UA_Log_Stdout;

static void stopHandler(int sign) {
	UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "Received Ctrl-C");
	running = 0;
}
static void
deleteSubscriptionCallback(UA_Client *client, UA_UInt32 subscriptionId, void *subscriptionContext) {
	UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "Subscription Id %u was deleted", subscriptionId);
}
static void
subscriptionInactivityCallback(UA_Client *client, UA_UInt32 subId, void *subContext) {
	UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "Inactivity for subscription %u", subId);
}
static void
handler_Changed(UA_Client *client, UA_UInt32 subId, void *subContext,
	UA_UInt32 monId, void *monContext, UA_DataValue *value) {
	if (UA_Variant_hasScalarType(&value->value, &UA_TYPES[UA_TYPES_INT32])) {
		UA_Int32 v = *(UA_Int32 *)value->value.data;
		cout << v << endl;
	}
	if (UA_Variant_hasArrayType(&value->value, &UA_TYPES[UA_TYPES_BYTE])) {
		framepixel *p = (framepixel *)value->value.data;

		Mat Main_frame;
		Main_frame.create(480, 640, CV_8UC3);
		Main_frame.data = p->x;


		namedWindow("Display window", WINDOW_AUTOSIZE);
		imshow("Display window", Main_frame);
		waitKey(1);
	}
}

static void
stateCallback(UA_Client *client, UA_ClientState clientState) {
	switch (clientState) {
	case UA_CLIENTSTATE_DISCONNECTED:
		UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "The client is disconnected");
		break;
	case UA_CLIENTSTATE_CONNECTED:
		UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "A TCP connection to the server is open");
		break;
	case UA_CLIENTSTATE_SECURECHANNEL:
		UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "A SecureChannel to the server is open");
		break;
	case UA_CLIENTSTATE_SESSION: {
		UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "A session with the server is open");
		/* A new session was created. We need to create the subscription. */
		/* Create a subscription */
		UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
		UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request,
			NULL, NULL, deleteSubscriptionCallback);

		if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
			UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "Create subscription succeeded, id %u", response.subscriptionId);
		else
			return;

		/* Add a MonitoredItem */
		//////////////////////////////////////////////////////////////////////
		UA_MonitoredItemCreateRequest monRequest =
			UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(1, "frame Variable"));
		UA_MonitoredItemCreateResult monResponse =
			UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
				UA_TIMESTAMPSTORETURN_BOTH,
				monRequest, NULL, handler_Changed, NULL);
		///////////////////////////////////////////////////////////////////
		UA_MonitoredItemCreateRequest monRequest1 =
		UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(1, "the.answer"));
		UA_MonitoredItemCreateResult monResponse1 =
			UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
				UA_TIMESTAMPSTORETURN_BOTH,
				monRequest1, NULL, handler_Changed, NULL);

			}
								 break;
	case UA_CLIENTSTATE_SESSION_RENEWED:
		UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "A session with the server is open (renewed)");
		/* The session was renewed. We don't need to recreate the subscription. */
		break;
	}
	return;
}

int main(int argc, char *argv[]) {

	UA_ClientConfig config = UA_ClientConfig_default;
	/* Set stateCallback */
	config.stateCallback = stateCallback;
	config.subscriptionInactivityCallback = subscriptionInactivityCallback;

	UA_Client *client = UA_Client_new(config);

	while (running) {
	UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); 
	//UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://192.168.0.7:4840");
	if (retval != UA_STATUSCODE_GOOD) {
		UA_Client_delete(client);
		return (int)retval;
		Sleep(1000);
		continue;
	}
	//////////////////////////////
	//UA_Variant value; /* Variants can hold scalar values and arrays of any type */
	//UA_Variant_init(&value);
	//UA_NodeId nodeId =UA_NODEID_STRING(1, "the.answer");
	//retval = UA_Client_readValueAttribute(client, nodeId, &value);
	//{
	//	UA_Int32 v = *(UA_Int32 *)value.data;
	//	cout << v<<endl;
	
	
	///////////////////////////////

	//UA_Variant_init(&value);
	//nodeId = UA_NODEID_STRING(1, "frame Variable");
	//retval = UA_Client_readValueAttribute(client, nodeId, &value);
	//
	//	framepixel *p = (framepixel *)value.data;

	//	Mat Main_frame;
	//	Main_frame.create(234, 263, CV_8UC1);
	//	Main_frame.data = p->x;


	//	namedWindow("Display window", WINDOW_AUTOSIZE);
	//	imshow("Display window", Main_frame);

	//	waitKey(1);

		UA_Client_runAsync(client, 1000);
	}
	
	system("pause");
	UA_Client_disconnect(client);
	UA_Client_delete(client);
	return (int)UA_STATUSCODE_GOOD;
}