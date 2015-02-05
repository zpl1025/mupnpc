/******************************************************************
*
*	CyberLink for C
*
*	Copyright (C) Satoshi Konno 2005
*
*       Copyright (C) 2006 Nokia Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
*	File: cnotify_request.c
*
*	Revision:
*
*	07/07/05
*		- first revision
*	03/13/08
*		- Changed mupnp_upnp_event_notify_request_setpropertysetnode() using void parameter instead of CgService not to conflict the prototype defines.
*
******************************************************************/

#include <mupnp/event/notify.h>
#include <mupnp/event/event.h>
#include <mupnp/event/subscriber.h>
#include <mupnp/event/property.h>
#include <mupnp/control/control.h>
#include <mupnp/statevariable.h>
#include <mupnp/util/log.h>

/****************************************
* CG_UPNP_NOUSE_SUBSCRIPTION (Begin)
****************************************/

#if !defined(CG_UPNP_NOUSE_SUBSCRIPTION)

/****************************************
* Static Function Prototype
****************************************/

static CgXmlNode *mupnp_upnp_event_notify_request_createpropertysetnode(CgUpnpService* service, CgUpnpStateVariable *statVar);

#define mupnp_upnp_event_notify_request_getpropertylistonly(notifyReq) ((CgUpnpPropertyList *)mupnp_soap_request_getuserdata(notifyReq))

/****************************************
* mupnp_upnp_event_notify_request_new
****************************************/

CgUpnpNotifyRequest *mupnp_upnp_event_notify_request_new()
{
	CgUpnpNotifyRequest *notifyReq;
	CgUpnpPropertyList *propList;

	mupnp_log_debug_l4("Entering...\n");

	notifyReq = mupnp_soap_request_new();

	propList = mupnp_upnp_propertylist_new();
	mupnp_upnp_event_notify_request_setpropertylist(notifyReq, propList);

	mupnp_log_debug_l4("Leaving...\n");

	return notifyReq;
}

/****************************************
* mupnp_upnp_event_notify_request_delete
****************************************/

void mupnp_upnp_event_notify_request_delete(CgUpnpNotifyRequest *notifyReq)
{
	CgUpnpPropertyList *propList;

	mupnp_log_debug_l4("Entering...\n");

	propList = mupnp_upnp_event_notify_request_getpropertylistonly(notifyReq);
	mupnp_upnp_propertylist_delete(propList);

	mupnp_soap_request_delete(notifyReq);

	mupnp_log_debug_l4("Leaving...\n");
}

/****************************************
* mupnp_upnp_event_notify_request_clear
****************************************/

void mupnp_upnp_event_notify_request_clear(CgUpnpNotifyRequest *notifyReq)
{
	CgUpnpPropertyList *propList;

	mupnp_log_debug_l4("Entering...\n");

	propList = mupnp_upnp_event_notify_request_getpropertylistonly(notifyReq);
	mupnp_upnp_propertylist_clear(propList);

	mupnp_soap_request_clear(notifyReq);

	mupnp_log_debug_l4("Leaving...\n");
}

/****************************************
* mupnp_upnp_event_notify_request_setsid
****************************************/

void mupnp_upnp_event_notify_request_setsid(CgUpnpNotifyRequest *soapReq, char *sid)
{
	char buf[CG_UPNP_SUBSCRIPTION_SID_HEADER_SIZE];

	mupnp_log_debug_l4("Entering...\n");

	mupnp_http_packet_setheadervalue((CgHttpPacket*)(soapReq->httpReq), CG_HTTP_SID, mupnp_upnp_event_subscription_tosidheaderstring(sid, buf, sizeof(buf)));

	mupnp_log_debug_l4("Leaving...\n");
}

/****************************************
* mupnp_upnp_event_notify_request_setpropertysetnode
****************************************/
BOOL mupnp_upnp_event_notify_request_setpropertysetnode(CgUpnpNotifyRequest *notifyReq, CgUpnpSubscriber *sub, /* CgUpnpService */void *pservice, CgUpnpStateVariable *statVar)
{
	CgHttpRequest *httpReq;
	CgXmlNode *propSetNode;
	CgUpnpService* service;
  char server[CG_UPNP_SEVERNAME_MAXLEN];

	mupnp_log_debug_l4("Entering...\n");

	service = (CgUpnpService *)pservice;

	httpReq = mupnp_soap_request_gethttprequest(notifyReq);

	mupnp_http_request_setmethod(httpReq, CG_HTTP_NOTIFY);
	mupnp_http_request_setconnection(httpReq, CG_HTTP_CLOSE);
	mupnp_http_request_seturi(httpReq, mupnp_upnp_subscriber_getdeliverypath(sub));
	mupnp_http_request_sethost(httpReq, mupnp_upnp_subscriber_getdeliveryhost(sub), mupnp_upnp_subscriber_getdeliveryport(sub));
  mupnp_upnp_getservername(server, sizeof(server));
  mupnp_http_packet_setheadervalue((CgHttpPacket*)httpReq,
                                  CG_HTTP_SERVER,
                                  server);
	mupnp_upnp_event_notify_request_setnt(notifyReq, CG_UPNP_NT_EVENT);
	mupnp_upnp_event_notify_request_setnts(notifyReq, CG_UPNP_NTS_PROPCHANGE);
	mupnp_upnp_event_notify_request_setsid(notifyReq, mupnp_upnp_subscriber_getsid(sub));
	mupnp_upnp_event_notify_request_setseq(notifyReq, mupnp_upnp_subscriber_getnotifycount(sub));

	propSetNode = mupnp_upnp_event_notify_request_createpropertysetnode(service, statVar);
	mupnp_soap_request_setcontent(notifyReq, propSetNode);
	mupnp_xml_node_delete(propSetNode);

	mupnp_log_debug_l4("Leaving...\n");

	return TRUE;
}

/****************************************
* mupnp_upnp_event_notify_request_createpropertysetnode
****************************************/

static CgXmlNode *mupnp_upnp_event_notify_request_createpropertysetnode(CgUpnpService* service, CgUpnpStateVariable *statVar)
{
	CgXmlNode *propSetNode;
	CgXmlNode *propNode;
	CgXmlNode *varNode;

	mupnp_log_debug_l4("Entering...\n");

	propSetNode = mupnp_xml_node_new();
	mupnp_xml_node_setname(propSetNode, CG_UPNP_NOTIFY_XMLNS CG_SOAP_DELIM CG_UPNP_NOTIFY_PROPERTYSET);
	mupnp_xml_node_setnamespace(propSetNode, CG_UPNP_NOTIFY_XMLNS, CG_UPNP_SUBSCRIPTION_XMLNS);

	if (service) {
		for (statVar = mupnp_upnp_service_getstatevariables(service); statVar != NULL; statVar = mupnp_upnp_statevariable_next(statVar)) {
			if (!mupnp_upnp_statevariable_issendevents(statVar))
				continue;
			propNode = mupnp_xml_node_new();
			if (!propNode)
				continue;
			mupnp_xml_node_setname(propNode, CG_UPNP_NOTIFY_XMLNS CG_SOAP_DELIM CG_UPNP_NOTIFY_PROPERTY);
			mupnp_xml_node_addchildnode(propSetNode, propNode);
			varNode = mupnp_xml_node_new();
			if (!varNode) {
				mupnp_xml_node_delete(propNode);
				continue;
			}
			mupnp_xml_node_setname(varNode, mupnp_upnp_statevariable_getname(statVar));
			mupnp_xml_node_setvalue(varNode, mupnp_upnp_statevariable_getvalue(statVar));
			mupnp_xml_node_addchildnode(propNode, varNode);
		}
	}
	else if (statVar) {
		propNode = mupnp_xml_node_new();
		if (propNode) {
			mupnp_xml_node_setname(propNode, CG_UPNP_NOTIFY_XMLNS CG_SOAP_DELIM CG_UPNP_NOTIFY_PROPERTY);
			mupnp_xml_node_addchildnode(propSetNode, propNode);
			varNode = mupnp_xml_node_new();
			if (varNode) {
				mupnp_xml_node_setname(varNode, mupnp_upnp_statevariable_getname(statVar));
				mupnp_xml_node_setvalue(varNode, mupnp_upnp_statevariable_getvalue(statVar));
				mupnp_xml_node_addchildnode(propNode, varNode);
			}
			else
				mupnp_xml_node_delete(propNode);
		}
	}

	mupnp_log_debug_l4("Leaving...\n");

	return propSetNode;
}

/****************************************
* mupnp_upnp_event_notify_request_getvariablenode
****************************************/

CgXmlNode *mupnp_upnp_event_notify_request_getvariablenode(CgUpnpNotifyRequest *nofityReq)
{
	CgXmlNode *propSetNode;
	CgXmlNode *propNode;

	mupnp_log_debug_l4("Entering...\n");

	propSetNode = mupnp_upnp_event_notify_request_getpropertysetnode(nofityReq);
	if (propSetNode == NULL)
		return NULL;
	if (mupnp_xml_node_haschildnodes(propSetNode) == FALSE)
		return NULL;

	propNode = mupnp_xml_node_getchildnodes(propSetNode);
	if (propNode == NULL)
		return NULL;
	if (mupnp_xml_node_haschildnodes(propNode) == FALSE)
		return NULL;

	mupnp_log_debug_l4("Leaving...\n");

	return mupnp_xml_node_getchildnodes(propNode);
}

/****************************************
* mupnp_upnp_property_createfromnode
****************************************/

static CgUpnpProperty *mupnp_upnp_property_createfromnode(CgXmlNode *varNode)
{
	CgUpnpProperty *prop;
	char *varName;
	char *varValue;
	ssize_t colonIdx;

	mupnp_log_debug_l4("Entering...\n");

	prop = mupnp_upnp_property_new();
	if (varNode == NULL)
		return prop;

	// remove the event namespace
	varName = mupnp_xml_node_getname(varNode);
	colonIdx = mupnp_strstr(varName, ":");
	if (0 <= colonIdx)
		varName = varName + colonIdx + 1;
	varValue = mupnp_xml_node_getvalue(varNode);
	mupnp_upnp_property_setname(prop, varName);
	mupnp_upnp_property_setvalue(prop, varValue);

	mupnp_log_debug_l4("Leaving...\n");

	return prop;
}

/****************************************
* mupnp_upnp_event_notify_request_getpropertylist
****************************************/

CgUpnpPropertyList *mupnp_upnp_event_notify_request_getpropertylist(CgUpnpNotifyRequest *notifyReq)
{
	CgUpnpPropertyList *propList;
	CgXmlNode *propSetNode;
	CgXmlNode *propNode;
	CgXmlNode *varNode;
	CgUpnpProperty *prop;
	const char *sid;
	size_t seq;

	mupnp_log_debug_l4("Entering...\n");

  sid = mupnp_upnp_event_notify_request_getsid(notifyReq);
	seq = mupnp_upnp_event_notify_request_getseq(notifyReq);

	propList = mupnp_upnp_event_notify_request_getpropertylistonly(notifyReq);
	mupnp_upnp_propertylist_clear(propList);

	propSetNode = mupnp_soap_request_getrootnoode(notifyReq);
	if (propSetNode == NULL)
		return propList;

	for (propNode = mupnp_xml_node_getchildnodes(propSetNode); propNode != NULL; propNode = mupnp_xml_node_next(propNode)) {
		varNode = mupnp_xml_node_getchildnodes(propNode);
		prop = mupnp_upnp_property_createfromnode(varNode);
		mupnp_upnp_property_setsid(prop, sid);
		mupnp_upnp_property_setseq(prop, seq);
		mupnp_upnp_propertylist_add(propList, prop);
	}

	mupnp_log_debug_l4("Leaving...\n");

	return propList;
}

/****************************************
* CG_UPNP_NOUSE_SUBSCRIPTION (End)
****************************************/

#endif