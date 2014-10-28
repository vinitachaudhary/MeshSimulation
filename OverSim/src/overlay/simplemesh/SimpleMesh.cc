//
// Copyright (C) 2010 DenaCast All Rights Reserved.
// http://www.denacast.com
// The DenaCast was designed and developed at the DML(Digital Media Lab http://dml.ir/)
// under supervision of Dr. Behzad Akbari (http://www.modares.ac.ir/ece/b.akbari)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

/**
 * @file SimpleMesh.cc
 * @author Yasser Seyyedi, Behnam Ahmadifar
 */

#include <SimpleInfo.h>
#include "SimpleMesh.h"
#include <GlobalStatistics.h>
#include "DenaCastTrackerMessage_m.h"
#include "VideoMessage_m.h"
#include "SimpleMeshMessage_m.h"


Define_Module(SimpleMesh);

void SimpleMesh::initializeOverlay(int stage)
{
	if (stage != MIN_STAGE_OVERLAY)
		return;
	isRegistered = false;
    if(globalNodeList->getPeerInfo(thisNode.getAddress())->getTypeID() == 2)
    	isSource = true;
    else
    	isSource = false;
    passiveNeighbors = par("passiveNeighbors");
    activeNeighbors = par("activeNeighbors");
	neighborNotificationPeriod = par("neighborNotificationPeriod");
	neighborNum = passiveNeighbors + activeNeighbors;

	videoAverageRate = par("videoAverageRate");
	adaptiveNeighboring = par("adaptiveNeighboring");
	serverGradualNeighboring = par("serverGradualNeighboring");
	if(adaptiveNeighboring)
		neighborNum = (int)(upBandwidth/(videoAverageRate*1024) + 1);
	if(isSource)
	{
		neighborNum = activeNeighbors;
		getParentModule()->getParentModule()->setName("CDN-Server");
	}
    DenaCastOverlay::initializeOverlay(stage);
	WATCH(neighborNum);
	WATCH(downBandwidth);
	WATCH(upBandwidth);

	stat_TotalDownBandwidthUsage = 0;
	stat_TotalUpBandwidthUsage = 0;

    stat_joinREQ = 0;
	stat_joinREQBytesSent = 0;
	stat_joinRSP = 0;
	stat_joinRSPBytesSent = 0;
	stat_joinACK = 0;
	stat_joinACKBytesSent = 0;
	stat_joinDNY = 0;
	stat_joinDNYBytesSent = 0;
	stat_disconnectMessages = 0;
	stat_disconnectMessagesBytesSent = 0;
	stat_addedNeighbors = 0;
	stat_nummeshJoinRequestTimer = 0;

	//===============edited by vinita================
	numOfPowerResponsesExpected=0;
}

void SimpleMesh::joinOverlay()
{
	trackerAddress  = *globalNodeList->getRandomAliveNode(1);
	remainNotificationTimer = new cMessage ("remainNotificationTimer");
	scheduleAt(simTime()+neighborNotificationPeriod,remainNotificationTimer);
	std::stringstream ttString;
	ttString << thisNode;
	getParentModule()->getParentModule()->getDisplayString().setTagArg("tt",0,ttString.str().c_str());
	if(!isSource)
	{
		meshJoinRequestTimer = new cMessage("meshJoinRequestTimer");
		scheduleAt(simTime(),meshJoinRequestTimer);
		neighborSelectionTimer = new cMessage("neighborSelectionTimer");
		moveRequestTimer=new cMessage("moveRequestTimer");
		scheduleAt(simTime()+2,moveRequestTimer);
		moveAcceptTimer= new cMessage("moveAcceptTimer");
		parentReplaceTimer = new cMessage("parentReplaceTimer");
	}
	else
	{
		serverNeighborTimer = new cMessage("serverNeighborTimer");
		if(serverGradualNeighboring)
			scheduleAt(simTime()+uniform(15,25),serverNeighborTimer);
		selfRegister();
	}
	setOverlayReady(true);
}

void SimpleMesh::handleTimerEvent(cMessage* msg)
{
	if(msg == meshJoinRequestTimer)
	{
		//if(LV->neighbors.size() < activeNeighbors)
		if (LV->neighbourDownBandwidthReceived.size() < activeNeighbors)
		{
			DenaCastTrackerMessage* NeighborReq = new DenaCastTrackerMessage("NeighborReq");
			NeighborReq->setCommand(NEIGHBOR_REQUEST);
			//NeighborReq->setNeighborSize(activeNeighbors - LV->neighbors.size());
			NeighborReq->setNeighborSize(activeNeighbors - LV->neighbourDownBandwidthReceived.size());
			NeighborReq->setSrcNode(thisNode);
			NeighborReq->setIsServer(false);

			sendMessageToUDP(trackerAddress,NeighborReq);
			scheduleAt(simTime()+2,meshJoinRequestTimer);
		}
		else
			scheduleAt(simTime()+20,meshJoinRequestTimer);
	}
	else if(msg == remainNotificationTimer)
	{
		DenaCastTrackerMessage* remainNotification = new DenaCastTrackerMessage("remainNotification");
		remainNotification->setCommand(REMAIN_NEIGHBOR);
		//remainNotification->setRemainNeighbor(activeNeighbors - LV->neighbors.size() + passiveNeighbors);
		remainNotification->setRemainNeighbor(activeNeighbors - LV->neighbourDownBandwidthReceived.size() -
				LV->neighbourUpBandwidthAllotment.size() + passiveNeighbors);
		remainNotification->setSrcNode(thisNode);
		sendMessageToUDP(trackerAddress,remainNotification);
		scheduleAt(simTime()+2,remainNotificationTimer);

	}
	else if (msg == serverNeighborTimer)
	{
		if(isSource)
		{
			if(neighborNum < passiveNeighbors + activeNeighbors)
			{
				neighborNum +=1;
				cancelEvent(remainNotificationTimer);
				scheduleAt(simTime(),remainNotificationTimer);
				scheduleAt(simTime()+uniform(15,25),serverNeighborTimer);
			}
		}
//		else
//		{
//			neighborNum +=1;
//			remainedNeighbor = neighborNum - neighbors.size();
//			cancelEvent(remainNotificationTimer);
//			scheduleAt(simTime(),remainNotificationTimer);
//		}
	}

	//==============edited by vinita==================
	else if (msg == neighborSelectionTimer) {
		double bandwidthRequested=0.0, bandwidthUsed;
		neighborPowerMap neighbourPower;

		SimpleMeshMessage* joinDeny = new SimpleMeshMessage("joinDeny");
		joinDeny->setCommand(JOIN_DENY);
		joinDeny->setSrcNode(thisNode);
		joinDeny->setBitLength(SIMPLEMESHMESSAGE_L(msg));

		while (!neighborPowerBuffer.empty() && bandwidthRequested<videoAverageRate*1024) {
			neighbourPower=neighborPowerBuffer.top();
			neighborPowerBuffer.pop();
			//std::cout<<neighbourPower.power<<endl;

			if (neighbourPower.power>0) {
				bandwidthUsed=std::min(neighbourPower.residualUpBandwidth,videoAverageRate*1024-bandwidthRequested);
				//LV->neighbors.push_back(neighbourPower.tAddress);
				LV->neighbourDownBandwidthReceived[neighbourPower.tAddress]=bandwidthUsed;
				LV->neighbourDelay[neighbourPower.tAddress]=neighbourPower.sourceToEndDelay;
				//LV->setSourceToEndDelay();

				bandwidthRequested+=bandwidthUsed;

				if(!isRegistered)
					selfRegister();
				showOverlayNeighborArrow(neighbourPower.tAddress, false,
										 "m=m,50,0,50,0;ls=red,1");

				SimpleMeshMessage* joinAck = new SimpleMeshMessage("joinAck");
				joinAck->setCommand(JOIN_ACK);
				joinAck->setSrcNode(thisNode);
				joinAck->setBitLength(SIMPLEMESHMESSAGE_L(msg));

				cMsgPar *bandwidthUsedMsg= new cMsgPar("upBandwidthUsed");
				bandwidthUsedMsg->setDoubleValue(bandwidthUsed);
				joinAck->addPar(bandwidthUsedMsg);

				sendMessageToUDP(neighbourPower.tAddress,joinAck);

				stat_joinACK += 1;
				stat_joinACKBytesSent += joinAck->getByteLength();
			}
			else
			{
				sendMessageToUDP(neighbourPower.tAddress,joinDeny->dup());

				stat_joinDNY += 1;
				stat_joinDNYBytesSent += joinDeny->getByteLength();
			}
		}
		while (!neighborPowerBuffer.empty()) {
			neighbourPower=neighborPowerBuffer.top();
			neighborPowerBuffer.pop();
			sendMessageToUDP(neighbourPower.tAddress,joinDeny->dup());

			stat_joinDNY += 1;
			stat_joinDNYBytesSent += joinDeny->getByteLength();
		}
		delete joinDeny;
	}

	else if(msg==moveRequestTimer) {
		if (LV->getResidualUpBandwidth()>videoAverageRate*1024) {
			SimpleMeshMessage* moveRequest = new SimpleMeshMessage("moveRequest");
			moveRequest->setCommand(MOVE_REQUEST);
			moveRequest->setSrcNode(thisNode);
			moveRequest->setBitLength(SIMPLEMESHMESSAGE_L(msg));

			cMsgPar *UpBandwidth= new cMsgPar("UpBandwidth");
			UpBandwidth->setDoubleValue(LV->getUpBandwidth());
			moveRequest->addPar(UpBandwidth);

			cMsgPar *TTL= new cMsgPar ("TTL");
			TTL->setLongValue(2);
			moveRequest->addPar(TTL);

			std::map <TransportAddress,double>::iterator it;
			for (it=LV->neighbourDownBandwidthReceived.begin(); it!=LV->neighbourDownBandwidthReceived.end(); ++it) {
				sendMessageToUDP(it->first,moveRequest->dup());
			}
			delete moveRequest;
			scheduleAt(simTime()+1,moveAcceptTimer);
		}
		else
			scheduleAt(simTime()+10,moveRequestTimer);
	}
	else if (msg==moveAcceptTimer) {
		if (LV->getResidualUpBandwidth()>videoAverageRate*1024 && !neighborSEDBuffer.empty()) {
			std::map <TransportAddress,double>::iterator it, minIt;

			minIt=neighborSEDBuffer.begin();
			for (it=neighborSEDBuffer.begin(); it!=neighborSEDBuffer.end(); ++it) {
				if (it->second<minIt->second)
					minIt=it;
			}
			oldParents=LV->neighbourDownBandwidthReceived;
			//LV->neighbors.push_back(minIt->first);
			LV->neighbourUpBandwidthAllotment[minIt->first]=videoAverageRate*1024;

			SimpleMeshMessage* moveAccept = new SimpleMeshMessage("moveAccept");
			moveAccept->setCommand(MOVE_ACCEPT);
			moveAccept->setSrcNode(thisNode);
			moveAccept->setBitLength(SIMPLEMESHMESSAGE_L(msg));
			moveAccept->setTimestamp();

			cMsgPar *bandwidthAlloted= new cMsgPar("upBandwidthAlloted");
			bandwidthAlloted->setDoubleValue(videoAverageRate*1000);
			moveAccept->addPar(bandwidthAlloted);

			cMsgPar *SourceEndDelayMsg= new cMsgPar("SourceToEndDelay");
			SourceEndDelayMsg->setDoubleValue(LV->getSourceToEndDelay());
			moveAccept->addPar(SourceEndDelayMsg);

			sendMessageToUDP(minIt->first,moveAccept);
		}
	}
	else if (msg==parentReplaceTimer) {
		if (LV->neighbourDownBandwidthReceived.size() > oldParents.size()) {
			std::map <TransportAddress,double>::iterator it;
			for (it=oldParents.begin(); it!=oldParents.end(); ++it) {

				deleteOverlayNeighborArrow(it->first);
				LV->neighbourDownBandwidthReceived.erase(it->first);
				LV->neighbourDelay.erase(it->first);

				SimpleMeshMessage* disconnect = new SimpleMeshMessage("disconnectChild");
				disconnect->setCommand(DISCONNECT_CHILD);
				disconnect->setSrcNode(thisNode);
				disconnect->setBitLength(SIMPLEMESHMESSAGE_L(msg));
				sendMessageToUDP(it->first,disconnect);
			}
			oldParents.clear();
		}
	}
	else
		DenaCastOverlay::handleAppMessage(msg);
}
void SimpleMesh::handleUDPMessage(BaseOverlayMessage* msg)
{
	if (dynamic_cast<DenaCastTrackerMessage*>(msg) != NULL)
	{
		DenaCastTrackerMessage* trackerMsg = check_and_cast<DenaCastTrackerMessage*>(msg);

		//===============edit start by vinita========================
		if(trackerMsg->getCommand() == NEIGHBOR_RESPONSE)
		{
			SimpleMeshMessage* joinRequest = new SimpleMeshMessage("joinRequest");
			joinRequest->setCommand(JOIN_REQUEST);
			joinRequest->setSrcNode(thisNode);
			joinRequest->setBitLength(SIMPLEMESHMESSAGE_L(msg));
			int limit = 0;
			/*if(trackerMsg->getNeighborsArraySize() < activeNeighbors - LV->neighbors.size() )
				limit = trackerMsg->getNeighborsArraySize();
			else
				limit = activeNeighbors - LV->neighbors.size();*/

			if(trackerMsg->getNeighborsArraySize() < activeNeighbors - LV->neighbourDownBandwidthReceived.size() )
				limit = trackerMsg->getNeighborsArraySize();
			else
				limit = activeNeighbors - LV->neighbourDownBandwidthReceived.size();

			//limit = std::min(trackerMsg->getNeighborsArraySize(),(activeNeighbors - LV->neighbourDownBandwidthReceived.size()) );

			numOfPowerResponsesExpected=limit;
			for(unsigned int i=0 ; i <trackerMsg->getNeighborsArraySize() ; i++) {
				//if(limit-- > 0 && !isInVector(trackerMsg->getNeighbors(i),LV->neighbors))
				if (limit-- > 0 && LV->neighbourDownBandwidthReceived.find(trackerMsg->getNeighbors(i)) == LV->neighbourDownBandwidthReceived.end())
					sendMessageToUDP(trackerMsg->getNeighbors(i),joinRequest->dup());
			}

			cancelEvent(neighborSelectionTimer);
			scheduleAt(simTime()+1,neighborSelectionTimer);

			delete joinRequest;
		}

		delete trackerMsg;
	}
	else if (dynamic_cast<SimpleMeshMessage*>(msg) != NULL)
	{
		SimpleMeshMessage* simpleMeshmsg = check_and_cast<SimpleMeshMessage*>(msg);
		if (simpleMeshmsg->getCommand() == JOIN_REQUEST)
		{
			//if(LV->neighbors.size() < neighborNum)
			if (LV->neighbourUpBandwidthAllotment.size() < passiveNeighbors)
			{
				//LV->neighbors.push_back(simpleMeshmsg->getSrcNode());
				SimpleMeshMessage* joinResponse = new SimpleMeshMessage("joinResponse");
				joinResponse->setCommand(JOIN_RESPONSE);
				joinResponse->setSrcNode(thisNode);
				joinResponse->setBitLength(SIMPLEMESHMESSAGE_L(msg));

				joinResponse->setTimestamp();
				cMsgPar *residualBwMsg= new cMsgPar("residualUpBandwidth");
				residualBwMsg->setDoubleValue(LV->getResidualUpBandwidth());
				joinResponse->addPar(residualBwMsg);

				cMsgPar *SourceEndDelayMsg= new cMsgPar("SourceToEndDelay");
				if(isSource)
					SourceEndDelayMsg->setDoubleValue(0.0);
				else {
					SourceEndDelayMsg->setDoubleValue(LV->getSourceToEndDelay());
				}
				joinResponse->addPar(SourceEndDelayMsg);

				sendMessageToUDP(simpleMeshmsg->getSrcNode(),joinResponse);

				stat_joinRSP += 1;
				stat_joinRSPBytesSent += joinResponse->getByteLength();
			}
			else
			{
				SimpleMeshMessage* joinDeny = new SimpleMeshMessage("joinDeny");
				joinDeny->setCommand(JOIN_DENY);
				joinDeny->setSrcNode(thisNode);
				joinDeny->setBitLength(SIMPLEMESHMESSAGE_L(msg));
				sendMessageToUDP(simpleMeshmsg->getSrcNode(),joinDeny);

				stat_joinDNY += 1;
				stat_joinDNYBytesSent += joinDeny->getByteLength();
			}
		}
		else if (simpleMeshmsg->getCommand() == JOIN_RESPONSE)
		{
			SimpleMeshMessage* joinDeny = new SimpleMeshMessage("joinDeny");
			joinDeny->setCommand(JOIN_DENY);
			joinDeny->setSrcNode(thisNode);
			joinDeny->setBitLength(SIMPLEMESHMESSAGE_L(msg));

			bool deny=false;
			//if(LV->neighbors.size() < neighborNum )
			if (LV->neighbourDownBandwidthReceived.size() < activeNeighbors)
			{
				if (numOfPowerResponsesExpected>0) {
					neighborPowerMap neighbourPower;
					neighbourPower.tAddress=simpleMeshmsg->getSrcNode();
					neighbourPower.sourceToEndDelay = (simTime()-simpleMeshmsg->getTimestamp()).dbl();
					if (simpleMeshmsg->hasPar("residualUpBandwidth") && simpleMeshmsg->hasPar("SourceToEndDelay")) {
						neighbourPower.residualUpBandwidth = simpleMeshmsg->par("residualUpBandwidth").doubleValue();
						neighbourPower.sourceToEndDelay += simpleMeshmsg->par("SourceToEndDelay").doubleValue();
						neighbourPower.power=neighbourPower.residualUpBandwidth/neighbourPower.sourceToEndDelay;
						neighborPowerBuffer.push(neighbourPower);
					}
					else
						deny=true;
				}
				numOfPowerResponsesExpected--;

				if (deny==true) {
					sendMessageToUDP(simpleMeshmsg->getSrcNode(),joinDeny->dup());
					stat_joinDNY += 1;
					stat_joinDNYBytesSent += joinDeny->getByteLength();
				}

				else {
					if (numOfPowerResponsesExpected<=0) {
						cancelEvent(neighborSelectionTimer);
						scheduleAt(simTime(),neighborSelectionTimer);
					}
				}
			}
			else
			{
				sendMessageToUDP(simpleMeshmsg->getSrcNode(),joinDeny->dup());

				stat_joinDNY += 1;
				stat_joinDNYBytesSent += joinDeny->getByteLength();
			}
			delete joinDeny;
		}
		else if(simpleMeshmsg->getCommand() == JOIN_ACK)
		{
			double upBandwidthAlloted=0;
			if (simpleMeshmsg->hasPar("upBandwidthUsed"))
				upBandwidthAlloted=simpleMeshmsg->par("upBandwidthUsed").doubleValue();

			//if(LV->neighbors.size() <= neighborNum && LV->getResidualUpBandwidth()-upBandwidthAlloted>=0)
			if(LV->neighbourUpBandwidthAllotment.size() < passiveNeighbors && LV->getResidualUpBandwidth()-upBandwidthAlloted>=0)
			{
				if(!isRegistered)
					selfRegister();
				stat_addedNeighbors += 1;

				LV->neighbourUpBandwidthAllotment[simpleMeshmsg->getSrcNode()]=upBandwidthAlloted;

				//showOverlayNeighborArrow(simpleMeshmsg->getSrcNode(), false,"m=m,50,0,50,0;ls=red,1");
			}
			else
			{
				SimpleMeshMessage* joinDeny = new SimpleMeshMessage("joinDeny");
				joinDeny->setCommand(JOIN_DENY);
				joinDeny->setSrcNode(thisNode);
				joinDeny->setBitLength(SIMPLEMESHMESSAGE_L(msg));
				sendMessageToUDP(simpleMeshmsg->getSrcNode(),joinDeny);
				/*if(isInVector(simpleMeshmsg->getSrcNode(),LV->neighbors))
					deleteVector(simpleMeshmsg->getSrcNode(),LV->neighbors);*/
				//deleteOverlayNeighborArrow(simpleMeshmsg->getSrcNode());

				stat_joinDNY += 1;
				stat_joinDNYBytesSent += joinDeny->getByteLength();
			}
		}
		else if(simpleMeshmsg->getCommand() == JOIN_DENY)
		{
			/*if(isInVector(simpleMeshmsg->getSrcNode(),LV->neighbors))
				deleteVector(simpleMeshmsg->getSrcNode(),LV->neighbors);*/
			if (LV->neighbourDelay.find(simpleMeshmsg->getSrcNode())!=LV->neighbourDelay.end()) {
				LV->neighbourDelay.erase(simpleMeshmsg->getSrcNode());
				LV->neighbourDownBandwidthReceived.erase(simpleMeshmsg->getSrcNode());
				deleteOverlayNeighborArrow(simpleMeshmsg->getSrcNode());
			}
			else {
				numOfPowerResponsesExpected--;
				if (numOfPowerResponsesExpected<=0) {
					cancelEvent(neighborSelectionTimer);
					scheduleAt(simTime(),neighborSelectionTimer);
				}
			}
		}
		else if(simpleMeshmsg->getCommand() == DISCONNECT)
		{
			disconnectProcess(simpleMeshmsg->getSrcNode());
		}
		else if(simpleMeshmsg->getCommand() == DISCONNECT_PARENT)
		{
			if (LV->neighbourDownBandwidthReceived.find(simpleMeshmsg->getSrcNode()) != LV->neighbourDownBandwidthReceived.end()) {
				//if (LV->neighbourUpBandwidthAllotment.find(simpleMeshmsg->getSrcNode()) != LV->neighbourUpBandwidthAllotment.end()) {
				deleteOverlayNeighborArrow(simpleMeshmsg->getSrcNode());
				LV->neighbourDelay.erase(simpleMeshmsg->getSrcNode());
				LV->neighbourDownBandwidthReceived.erase(simpleMeshmsg->getSrcNode());
				cancelEvent(meshJoinRequestTimer);
				scheduleAt(simTime(),meshJoinRequestTimer);

				VideoMessage* videoMsg = new VideoMessage();
				videoMsg->setCommand(NEIGHBOR_LEAVE);
				videoMsg->setSrcNode(simpleMeshmsg->getSrcNode());
				send(videoMsg,"appOut");
				/*}
				else
					disconnectProcess(simpleMeshmsg->getSrcNode());*/
			}
		}
		else if(simpleMeshmsg->getCommand() == DISCONNECT_CHILD)
		{
			if (LV->neighbourUpBandwidthAllotment.find(simpleMeshmsg->getSrcNode()) != LV->neighbourUpBandwidthAllotment.end()) {
				//if (LV->neighbourDownBandwidthReceived.find(simpleMeshmsg->getSrcNode()) != LV->neighbourDownBandwidthReceived.end()) {
				LV->neighbourUpBandwidthAllotment.erase(simpleMeshmsg->getSrcNode());
				/*}
				else
					disconnectProcess(simpleMeshmsg->getSrcNode());*/
			}
		}
		else if (simpleMeshmsg->getCommand() == MOVE_REQUEST) {
			if (!isSource && simpleMeshmsg->hasPar("TTL") && simpleMeshmsg->hasPar("UpBandwidth")) {
				long TTL=simpleMeshmsg->par("TTL").longValue();
				TTL--;
				if (TTL>0) {
					simpleMeshmsg->par("TTL").setLongValue(TTL);
					std::map <TransportAddress,double>::iterator it;
					for (it=LV->neighbourDownBandwidthReceived.begin(); it!=LV->neighbourDownBandwidthReceived.end(); ++it) {
						sendMessageToUDP(it->first,simpleMeshmsg->dup());
					}
				}
				if (simpleMeshmsg->par("UpBandwidth").doubleValue()>LV->getUpBandwidth()) {
					SimpleMeshMessage* grantResponse = new SimpleMeshMessage("moveGrant");
					grantResponse->setCommand(MOVE_GRANT);
					grantResponse->setSrcNode(thisNode);
					grantResponse->setBitLength(SIMPLEMESHMESSAGE_L(msg));

					cMsgPar *sourceToEndDelay= new cMsgPar("sourceToEndDelay");
					sourceToEndDelay->setDoubleValue(LV->getSourceToEndDelay());
					grantResponse->addPar(sourceToEndDelay);

					sendMessageToUDP(simpleMeshmsg->getSrcNode(),grantResponse);
				}
			}
		}
		else if (simpleMeshmsg->getCommand() == MOVE_GRANT) {
			if (simpleMeshmsg->hasPar("sourceToEndDelay")) {
				neighborSEDBuffer[simpleMeshmsg->getSrcNode()]=simpleMeshmsg->par("sourceToEndDelay").doubleValue();
			}
		}
		else if (simpleMeshmsg->getCommand() == MOVE_ACCEPT) {
			//LV->neighbors.push_back(simpleMeshmsg->getSrcNode());
			showOverlayNeighborArrow(simpleMeshmsg->getSrcNode(), false,"m=m,50,0,50,0;ls=red,1");

			SimpleMeshParentListMessage* acceptAck = new SimpleMeshParentListMessage("acceptAck");
			acceptAck->setCommand(ACCEPT_ACK);
			acceptAck->setSrcNode(thisNode);
			acceptAck->setBitLength(SIMPLEMESHMESSAGE_L(msg));
			acceptAck->setParentsArraySize(LV->neighbourDownBandwidthReceived.size());

			std::map <TransportAddress,double>::iterator it;
			unsigned int i=0;
			for (it=LV->neighbourDownBandwidthReceived.begin(); it != LV->neighbourDownBandwidthReceived.end(); ++it, i++) {
				acceptAck->setParents(i,it->first);
			}
			sendMessageToUDP(simpleMeshmsg->getSrcNode(),acceptAck);

			if (simpleMeshmsg->hasPar("upBandwidthAlloted") && simpleMeshmsg->hasPar("SourceToEndDelay")) {
				LV->neighbourDownBandwidthReceived[simpleMeshmsg->getSrcNode()]=simpleMeshmsg->par("upBandwidthAlloted").doubleValue();
				LV->neighbourDelay[simpleMeshmsg->getSrcNode()]=simpleMeshmsg->par("SourceToEndDelay").doubleValue()+
						((simTime()-simpleMeshmsg->getTimestamp()).dbl());
			}
		}
		else if (simpleMeshmsg->getCommand() == REPLACE_ACK) {
			if (LV->neighbourDownBandwidthReceived.find(simpleMeshmsg->getSrcNode()) == LV->neighbourDownBandwidthReceived.end())
				showOverlayNeighborArrow(simpleMeshmsg->getSrcNode(), false,"m=m,50,0,50,0;ls=red,1");

			LV->neighbourDownBandwidthReceived[simpleMeshmsg->getSrcNode()]=simpleMeshmsg->par("upBandwidthAlloted").doubleValue();
			LV->neighbourDelay[simpleMeshmsg->getSrcNode()]=simpleMeshmsg->par("SourceToEndDelay").doubleValue() +
					(simTime()-simpleMeshmsg->getTimestamp()).dbl();

			/*if (!isInVector(simpleMeshmsg->getSrcNode(),LV->neighbors)) {
				LV->neighbors.push_back(simpleMeshmsg->getSrcNode());*/

			if (oldParents.size()>0) {
				std::map <TransportAddress,double>::iterator it=oldParents.begin();
				LV->neighbourDownBandwidthReceived.erase(it->first);
				LV->neighbourDelay.erase(it->first);

				deleteOverlayNeighborArrow(it->first);

				SimpleMeshMessage* disconnect = new SimpleMeshMessage("disconnectChild");
				disconnect->setCommand(DISCONNECT_CHILD);
				disconnect->setSrcNode(thisNode);
				disconnect->setBitLength(SIMPLEMESHMESSAGE_L(msg));
				sendMessageToUDP(it->first,disconnect);

				oldParents.erase(it);
			}
		}
		delete simpleMeshmsg;
	}
	else if (dynamic_cast<SimpleMeshParentListMessage*>(msg) != NULL) {
		SimpleMeshParentListMessage* simpleMeshParentListmsg = check_and_cast<SimpleMeshParentListMessage*>(msg);

		if (simpleMeshParentListmsg->getCommand() == ACCEPT_ACK && simpleMeshParentListmsg->getParentsArraySize()>0) {
			SimpleMeshReplaceMessage* replaceNode = new SimpleMeshReplaceMessage("replaceNode");
			replaceNode->setCommand(REPLACE);
			replaceNode->setSrcNode(thisNode);
			replaceNode->setBitLength(SIMPLEMESHMESSAGE_L(msg));
			replaceNode->setNodeToBeReplaced(simpleMeshParentListmsg->getSrcNode());

			for(unsigned int i=0 ; i <simpleMeshParentListmsg->getParentsArraySize() ; i++) {
				if (LV->neighbourDownBandwidthReceived.find(simpleMeshParentListmsg->getParents(i))!=LV->neighbourDownBandwidthReceived.end())
					oldParents.erase(simpleMeshParentListmsg->getParents(i));
				if(simpleMeshParentListmsg->getParents(i)!=thisNode)
					sendMessageToUDP(simpleMeshParentListmsg->getParents(i),replaceNode->dup());
			}

			scheduleAt(simTime()+2,parentReplaceTimer);
			delete replaceNode;
		}

		delete simpleMeshParentListmsg;
	}
	else if (dynamic_cast<SimpleMeshReplaceMessage*>(msg) != NULL) {
		SimpleMeshReplaceMessage* simpleMeshReplacemsg = check_and_cast<SimpleMeshReplaceMessage*>(msg);

		if (simpleMeshReplacemsg->getCommand() == REPLACE) {
			TransportAddress nodeToBeReplaced = simpleMeshReplacemsg->getNodeToBeReplaced();
			if (LV->neighbourUpBandwidthAllotment.find(nodeToBeReplaced) != LV->neighbourUpBandwidthAllotment.end()) {
				LV->neighbourUpBandwidthAllotment[simpleMeshReplacemsg->getSrcNode()]=LV->neighbourUpBandwidthAllotment[nodeToBeReplaced];
				LV->neighbourUpBandwidthAllotment.erase(nodeToBeReplaced);

				/*if (!isInVector(simpleMeshReplacemsg->getSrcNode(),LV->neighbors))
					LV->neighbors.push_back(simpleMeshReplacemsg->getSrcNode());*/

				SimpleMeshMessage* disconnect = new SimpleMeshMessage("disconnectParent");
				disconnect->setCommand(DISCONNECT_PARENT);
				disconnect->setSrcNode(thisNode);
				disconnect->setBitLength(SIMPLEMESHMESSAGE_L(msg));
				sendMessageToUDP(nodeToBeReplaced,disconnect);

				SimpleMeshMessage* replaceAck = new SimpleMeshMessage("replaceAck");
				replaceAck->setCommand(REPLACE_ACK);
				replaceAck->setSrcNode(thisNode);
				replaceAck->setBitLength(SIMPLEMESHMESSAGE_L(msg));

				replaceAck->setTimestamp();
				cMsgPar *upBwAlloted= new cMsgPar("upBandwidthAlloted");
				upBwAlloted->setDoubleValue(LV->neighbourUpBandwidthAllotment[simpleMeshReplacemsg->getSrcNode()]);
				replaceAck->addPar(upBwAlloted);

				cMsgPar *SourceEndDelayMsg= new cMsgPar("SourceToEndDelay");
				if(isSource)
					SourceEndDelayMsg->setDoubleValue(0.0);
				else {
					SourceEndDelayMsg->setDoubleValue(LV->getSourceToEndDelay());
				}
				replaceAck->addPar(SourceEndDelayMsg);

				sendMessageToUDP(simpleMeshReplacemsg->getSrcNode(),replaceAck);
			}
		}

		delete simpleMeshReplacemsg;
	}
	else
		DenaCastOverlay::handleUDPMessage(msg);
}
void SimpleMesh::handleNodeGracefulLeaveNotification()
{
	neighborNum = 0;
	selfUnRegister();
	std::cout << "time: " << simTime()<< "  "<<getParentModule()->getParentModule()->getFullName() <<  std::endl;

	SimpleMeshMessage* disconnectMsg = new SimpleMeshMessage("disconnect");
	disconnectMsg->setCommand(DISCONNECT);
	disconnectMsg->setSrcNode(thisNode);
	disconnectMsg->setBitLength(SIMPLEMESHMESSAGE_L(msg));
	/*for (unsigned int i=0; i != LV->neighbors.size(); i++)
	{
		sendMessageToUDP(LV->neighbors[i],disconnectMsg->dup());
		deleteOverlayNeighborArrow(LV->neighbors[i]);
		stat_disconnectMessages += 1;
		stat_disconnectMessagesBytesSent += disconnectMsg->getByteLength();
	}*/
	std::map <TransportAddress,double>::iterator it;
	for (it=LV->neighbourDownBandwidthReceived.begin(); it != LV->neighbourDownBandwidthReceived.end(); it++)
	{
		sendMessageToUDP(it->first,disconnectMsg->dup());
		deleteOverlayNeighborArrow(it->first);
		stat_disconnectMessages += 1;
		stat_disconnectMessagesBytesSent += disconnectMsg->getByteLength();
	}

	for (it=LV->neighbourUpBandwidthAllotment.begin(); it != LV->neighbourUpBandwidthAllotment.end(); it++)
	{
		sendMessageToUDP(it->first,disconnectMsg->dup());
		//deleteOverlayNeighborArrow(it->first);
		stat_disconnectMessages += 1;
		stat_disconnectMessagesBytesSent += disconnectMsg->getByteLength();
	}
	VideoMessage* videoMsg = new VideoMessage();
	videoMsg->setCommand(LEAVING);
	send(videoMsg,"appOut");
	delete disconnectMsg;
}

/*bool SimpleMesh::isInVector(TransportAddress& Node, std::vector <TransportAddress> &neighbors)
{
	for (unsigned int i=0; i!=neighbors.size(); i++)
	{
		if (neighbors[i] == Node)
		{
			return true;
		}
	}
	return false;
}
void SimpleMesh::deleteVector(TransportAddress Node,std::vector <TransportAddress> &neighbors)
{
	for (unsigned int i=0; i!=neighbors.size(); i++)
	{

		if(neighbors[i].isUnspecified())
		{
			neighbors.erase(neighbors.begin()+i,neighbors.begin()+1+i);
			break;
		}
	}
	if(Node.isUnspecified())
		return;
	for (unsigned int i=0; i!=neighbors.size(); i++)
	{

		if (Node == neighbors[i])
		{
			neighbors.erase(neighbors.begin()+i,neighbors.begin()+1+i);
			break;
		}
	}
}*/

void SimpleMesh::selfRegister()
{
	DenaCastTrackerMessage* selfReg = new DenaCastTrackerMessage("selfRegister");
	selfReg->setCommand(SELF_REGISTER);
	selfReg->setSrcNode(thisNode);
	selfReg->setIsServer(isSource);
	selfReg->setRemainNeighbor(passiveNeighbors);
	sendMessageToUDP(trackerAddress,selfReg);
	isRegistered = true;
}
void SimpleMesh::selfUnRegister()
{
	if(isRegistered)
	{
		DenaCastTrackerMessage* selfUnReg = new DenaCastTrackerMessage("selfUnRegister");
		selfUnReg->setCommand(SELF_UNREGISTER);
		selfUnReg->setSrcNode(thisNode);
		selfUnReg->setIsServer(isSource);
		sendMessageToUDP(trackerAddress,selfUnReg);
		isRegistered = false;
	}
}
void SimpleMesh::disconnectProcess(TransportAddress Node)
{
	//deleteVector(Node,LV->neighbors);
	deleteOverlayNeighborArrow(Node);

	if (LV->neighbourUpBandwidthAllotment.find(Node)!=LV->neighbourUpBandwidthAllotment.end()) {
		LV->neighbourUpBandwidthAllotment.erase(Node);
	}
	if(!isSource)
	{
		if (LV->neighbourDownBandwidthReceived.find(Node)!=LV->neighbourDownBandwidthReceived.end()) {
			LV->neighbourDelay.erase(Node);
			LV->neighbourDownBandwidthReceived.erase(Node);
			cancelEvent(meshJoinRequestTimer);
			scheduleAt(simTime(),meshJoinRequestTimer);
		}
	}
	VideoMessage* videoMsg = new VideoMessage();
	videoMsg->setCommand(NEIGHBOR_LEAVE);
	videoMsg->setSrcNode(Node);
	send(videoMsg,"appOut");
}
void SimpleMesh::finishOverlay()
{
	if(!isSource) {
    	cancelAndDelete(meshJoinRequestTimer);
    	cancelAndDelete(neighborSelectionTimer);
    	cancelAndDelete(moveRequestTimer);
    	cancelAndDelete(moveAcceptTimer);
    	cancelAndDelete(parentReplaceTimer);
	}
	else
		cancelAndDelete(serverNeighborTimer);
	globalStatistics->addStdDev("SimpleMesh: JOIN::REQ Messages", stat_joinREQ);
	globalStatistics->addStdDev("SimpleMesh: JOIN::REQ Bytes sent", stat_joinREQBytesSent);
	globalStatistics->addStdDev("SimpleMesh: JOIN::RSP Messages", stat_joinRSP);
	globalStatistics->addStdDev("SimpleMesh: JOIN::RSP Bytes sent", stat_joinRSPBytesSent);
	globalStatistics->addStdDev("SimpleMesh: JOIN::ACK Messages", stat_joinACK);
	globalStatistics->addStdDev("SimpleMesh: JOIN::ACK Bytes sent", stat_joinACKBytesSent);
	globalStatistics->addStdDev("SimpleMesh: JOIN::DNY Messages", stat_joinDNY);
	globalStatistics->addStdDev("SimpleMesh: JOIN::DNY Bytes sent", stat_joinDNYBytesSent);
	globalStatistics->addStdDev("SimpleMesh: DISCONNECT:IND Messages", stat_disconnectMessages);
	globalStatistics->addStdDev("SimpleMesh: DISCONNECT:IND Bytes sent", stat_disconnectMessagesBytesSent);
	globalStatistics->addStdDev("SimpleMesh: Neighbors added", stat_addedNeighbors);
	globalStatistics->addStdDev("SimpleMesh: Number of JOINRQ selfMessages", stat_nummeshJoinRequestTimer);
	globalStatistics->addStdDev("SimpleMesh: Download bandwidth", downBandwidth);
	globalStatistics->addStdDev("SimpleMesh: Upload bandwidth", upBandwidth);
	setOverlayReady(false);
}
