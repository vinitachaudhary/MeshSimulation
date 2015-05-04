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

// edited by vinita

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
    passiveNeighbors = intuniform(3,6);
    activeNeighbors = intuniform(6,12);
	neighborNotificationPeriod = par("neighborNotificationPeriod");
	neighborNum = passiveNeighbors + activeNeighbors;

	videoAverageRate = par("videoAverageRate");
	adaptiveNeighboring = par("adaptiveNeighboring");
	serverGradualNeighboring = par("serverGradualNeighboring");

	if(isSource)
	{
		neighborNum = activeNeighbors;
		getParentModule()->getParentModule()->setName("CDN-Server");
	}
    DenaCastOverlay::initializeOverlay(stage);
    if(adaptiveNeighboring)
    		neighborNum = (int)(upBandwidth/(videoAverageRate*1024) + 1);

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

	firstPeerAdded = false;
	stat_peerSelectionTime = 0.0;
	peerJoinTime = simTime().dbl();
	stat_peerSelectionToFirstChunkTime = 0.0;
	peerSelectionTime = 0.0;

	parentLeft = false;
	parentLeftTime = 0.0;
	sum_ParentReselectionTime =0.0;
	stat_parentReselectionTime = 0.0;
	countParentLeft = 0;

	numOfPowerResponsesExpected=0;
}

void SimpleMesh::joinOverlay()
{
	trackerAddress  = *globalNodeList->getRandomAliveNode(1);
	remainNotificationTimer = new cMessage ("remainNotificationTimer");
	aliveNotificationTimer = new cMessage ("aliveNotificationTimer");
	scheduleAt(simTime()+neighborNotificationPeriod,remainNotificationTimer);
	scheduleAt(simTime()+2,aliveNotificationTimer);
	checkNeighborTimer = new cMessage ("checkNeighborTimer");
	scheduleAt(simTime()+3,checkNeighborTimer);
	std::stringstream ttString;
	ttString << thisNode;
	getParentModule()->getParentModule()->getDisplayString().setTagArg("tt",0,ttString.str().c_str());
	if(!isSource)
	{
		meshJoinRequestTimer = new cMessage("meshJoinRequestTimer");
		scheduleAt(simTime(),meshJoinRequestTimer);
		neighborSelectionTimer = new cMessage("neighborSelectionTimer");
		moveRequestTimer=new cMessage("moveRequestTimer");
		scheduleAt(simTime()+10,moveRequestTimer);
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
		// Periodically check for new mesh neighbors if the node has available bandwidth
		if (LV->getCurrentDownBandwidth()<videoAverageRate*1024)
		{
			DenaCastTrackerMessage* NeighborReq = new DenaCastTrackerMessage("NeighborReq");
			NeighborReq->setCommand(NEIGHBOR_REQUEST);
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
		// Inform tracker periodically about own's existence and update its information
		DenaCastTrackerMessage* remainNotification = new DenaCastTrackerMessage("remainNotification");
		remainNotification->setCommand(REMAIN_NEIGHBOR);
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
	}

	else if (msg == neighborSelectionTimer) {
		// Timer to select neighbors based on power method - more upBandwidth and less delay.
		double bandwidthRequested=0.0, bandwidthUsed;
		neighborPowerMap neighbourPower;

		SimpleMeshMessage* joinDeny = new SimpleMeshMessage("joinDeny");
		joinDeny->setCommand(JOIN_DENY);
		joinDeny->setSrcNode(thisNode);
		joinDeny->setBitLength(SIMPLEMESHMESSAGE_L(msg));

		/* Select the neighbor from the remaining nodes, the one with highest power, till it has capacity.
		* Send join deny to the remaining nodes. */
		while (!neighborPowerBuffer.empty() && bandwidthRequested<videoAverageRate*1024) {
			neighbourPower=neighborPowerBuffer.top();
			neighborPowerBuffer.pop();

			if (neighbourPower.power>0) {
				bandwidthUsed=std::min(neighbourPower.residualUpBandwidth*0.4,videoAverageRate*1024-bandwidthRequested);
				LV->neighbourDownBandwidthReceived[neighbourPower.tAddress]=bandwidthUsed;
				LV->neighbourDelay[neighbourPower.tAddress]=neighbourPower.sourceToEndDelay;

				bandwidthRequested+=bandwidthUsed;

				if(!isRegistered) {
					selfRegister();
				}

				// Stat Collection for Startup Delay : Peer added for first time
				if (!firstPeerAdded) {
					firstPeerAdded = true;
					peerSelectionTime = simTime().dbl();
					stat_peerSelectionTime = peerSelectionTime - peerJoinTime;
				}
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

				// Stat Collection for Parent Reselection
				if (parentLeft) {
					parentLeft = false;
					sum_ParentReselectionTime+=simTime().dbl() - parentLeftTime;
				}
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
		// Sending join deny to remaining nodes due to unavailability of bandwidth
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
		/* Periodically send move request to ancestors.
		* The node with higher bandwidth tries to move closer to the source. */
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
		scheduleAt(simTime()+15,moveRequestTimer);
	}
	else if (msg==moveAcceptTimer) {
		/* Upon receiving several move grant messages, the node selects
		 * the node with minimum source-to-end delay and sends it move accept message. */
		if (LV->getResidualUpBandwidth()>videoAverageRate*1024 && !neighborSEDBuffer.empty()) {
			std::map <TransportAddress,double>::iterator it, minIt;

			minIt=neighborSEDBuffer.begin();
			for (it=neighborSEDBuffer.begin(); it!=neighborSEDBuffer.end(); ++it) {
				if (it->second<minIt->second)
					minIt=it;
			}
			oldParents=LV->neighbourDownBandwidthReceived;
			LV->neighbourUpBandwidthAllotment[minIt->first]=0.4*videoAverageRate*1024;

			SimpleMeshMessage* moveAccept = new SimpleMeshMessage("moveAccept");
			moveAccept->setCommand(MOVE_ACCEPT);
			moveAccept->setSrcNode(thisNode);
			moveAccept->setBitLength(SIMPLEMESHMESSAGE_L(msg));
			moveAccept->setTimestamp();

			cMsgPar *bandwidthAlloted= new cMsgPar("upBandwidthAlloted");
			bandwidthAlloted->setDoubleValue(0.4*videoAverageRate*1024);
			moveAccept->addPar(bandwidthAlloted);

			cMsgPar *SourceEndDelayMsg= new cMsgPar("SourceToEndDelay");
			SourceEndDelayMsg->setDoubleValue(LV->getSourceToEndDelay());
			moveAccept->addPar(SourceEndDelayMsg);

			sendMessageToUDP(minIt->first,moveAccept);
		}
	}
	else if (msg==parentReplaceTimer) {
		/* During the movement of node in overlay, if it has connected to new parents,
		*  disconnect with old parents.*/
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
	else if (msg==aliveNotificationTimer) {
		// Periodically send alive messages to neighbors along with updated information.
		std::map <TransportAddress,double>::iterator it;
		SimpleMeshMessage* alive = new SimpleMeshMessage("alive");
		alive->setCommand(ALIVE);
		alive->setSrcNode(thisNode);
		alive->setBitLength(SIMPLEMESHMESSAGE_L(msg));

		alive->setTimestamp();

		cMsgPar *SourceEndDelayMsg= new cMsgPar("SourceToEndDelay");
		if(isSource)
			SourceEndDelayMsg->setDoubleValue(0.0);
		else {
			SourceEndDelayMsg->setDoubleValue(LV->getSourceToEndDelay());
		}
		alive->addPar(SourceEndDelayMsg);

		for (it=LV->neighbourUpBandwidthAllotment.begin(); it!=LV->neighbourUpBandwidthAllotment.end(); ++it) {
			sendMessageToUDP(it->first,alive->dup());
		}
		for (it=LV->neighbourDownBandwidthReceived.begin(); it!=LV->neighbourDownBandwidthReceived.end(); ++it) {
			if (LV->neighbourUpBandwidthAllotment.find(it->first)==LV->neighbourUpBandwidthAllotment.end())
				sendMessageToUDP(it->first,alive->dup());
		}

		delete alive;
		scheduleAt(simTime()+2,aliveNotificationTimer);
	}
	else if (msg == checkNeighborTimer) {
		// Periodically flush out failed neighbors.
		std::map <TransportAddress,double>::iterator it, tempIt;
		it=neighborTimeOut.begin();

		while (it != neighborTimeOut.end()) {
			if (it->second < simTime().dbl()-4) {
				tempIt=it;
				++it;
				disconnectProcess(tempIt->first);
				neighborTimeOut.erase(tempIt);
			}
			else
				++it;
		}
		scheduleAt(simTime()+3,checkNeighborTimer);
	}
	else
		DenaCastOverlay::handleAppMessage(msg);
}
void SimpleMesh::handleUDPMessage(BaseOverlayMessage* msg)
{
	if (dynamic_cast<DenaCastTrackerMessage*>(msg) != NULL)
	{
		DenaCastTrackerMessage* trackerMsg = check_and_cast<DenaCastTrackerMessage*>(msg);

		if(trackerMsg->getCommand() == NEIGHBOR_RESPONSE)
		{
			// Send a join request to nodes received in message to locate neighbors.
			SimpleMeshMessage* joinRequest = new SimpleMeshMessage("joinRequest");
			joinRequest->setCommand(JOIN_REQUEST);
			joinRequest->setSrcNode(thisNode);
			joinRequest->setBitLength(SIMPLEMESHMESSAGE_L(msg));
			int limit = 0;

			if(trackerMsg->getNeighborsArraySize() < activeNeighbors - LV->neighbourDownBandwidthReceived.size() )
				limit = trackerMsg->getNeighborsArraySize();
			else
				limit = activeNeighbors - LV->neighbourDownBandwidthReceived.size();

			numOfPowerResponsesExpected=limit;
			for(unsigned int i=0 ; i <trackerMsg->getNeighborsArraySize() ; i++) {
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
			// If the node can accommodate more neighbors, send join response else join deny.
			if (LV->neighbourUpBandwidthAllotment.size() < passiveNeighbors)
			{
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
			/* If the node can accept more neighbors, it adds their information in a max priority queue, which
			 * prioritizes nodes based on power. Neighbors are finally selected when neighborSelectionTimer triggers.*/
			SimpleMeshMessage* joinDeny = new SimpleMeshMessage("joinDeny");
			joinDeny->setCommand(JOIN_DENY);
			joinDeny->setSrcNode(thisNode);
			joinDeny->setBitLength(SIMPLEMESHMESSAGE_L(msg));

			bool deny=false;
			if (LV->getCurrentDownBandwidth()<videoAverageRate*1024)
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
			/* If the node has accepted more neighbors than its capacity,
			* it sends a join deny else neighbor relationship is established. */
			double upBandwidthAlloted=0;
			if (simpleMeshmsg->hasPar("upBandwidthUsed"))
				upBandwidthAlloted=simpleMeshmsg->par("upBandwidthUsed").doubleValue();

			if(LV->neighbourUpBandwidthAllotment.size() < passiveNeighbors && LV->getResidualUpBandwidth()-upBandwidthAlloted>=0)
			{
				if(!isRegistered)
					selfRegister();
				stat_addedNeighbors += 1;
				LV->neighbourUpBandwidthAllotment[simpleMeshmsg->getSrcNode()]=upBandwidthAlloted;
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
		else if(simpleMeshmsg->getCommand() == JOIN_DENY)
		{
			// Remove the sender from corresponding lists.
			if (LV->neighbourDelay.find(simpleMeshmsg->getSrcNode())!=LV->neighbourDelay.end()) {
				LV->neighbourDelay.erase(simpleMeshmsg->getSrcNode());
				LV->neighbourDownBandwidthReceived.erase(simpleMeshmsg->getSrcNode());
				deleteOverlayNeighborArrow(simpleMeshmsg->getSrcNode());
			}
			else {
				numOfPowerResponsesExpected--;
				if (numOfPowerResponsesExpected<=0 && !isSource) {
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
			// Disconnect from the sender parent and locate new parent.
			if (LV->neighbourDownBandwidthReceived.find(simpleMeshmsg->getSrcNode()) != LV->neighbourDownBandwidthReceived.end()) {
				deleteOverlayNeighborArrow(simpleMeshmsg->getSrcNode());
				LV->neighbourDelay.erase(simpleMeshmsg->getSrcNode());
				LV->neighbourDownBandwidthReceived.erase(simpleMeshmsg->getSrcNode());
				cancelEvent(meshJoinRequestTimer);
				scheduleAt(simTime(),meshJoinRequestTimer);

				VideoMessage* videoMsg = new VideoMessage();
				videoMsg->setCommand(NEIGHBOR_LEAVE);
				videoMsg->setSrcNode(simpleMeshmsg->getSrcNode());
				send(videoMsg,"appOut");
			}
		}
		else if(simpleMeshmsg->getCommand() == DISCONNECT_CHILD)
		{
			// Disconnect from sender child.
			if (LV->neighbourUpBandwidthAllotment.find(simpleMeshmsg->getSrcNode()) != LV->neighbourUpBandwidthAllotment.end()) {
				LV->neighbourUpBandwidthAllotment.erase(simpleMeshmsg->getSrcNode());
			}
		}
		else if (simpleMeshmsg->getCommand() == ALIVE) {
			// Update the information of sender neighbor.
			neighborTimeOut[simpleMeshmsg->getSrcNode()]=simTime().dbl();

			if (LV->neighbourDelay.find(simpleMeshmsg->getSrcNode()) != LV->neighbourDelay.end()) {
				LV->neighbourDelay[simpleMeshmsg->getSrcNode()]=simpleMeshmsg->par("SourceToEndDelay").doubleValue()+
						((simTime()-simpleMeshmsg->getTimestamp()).dbl());
			}
		}
		else if (simpleMeshmsg->getCommand() == MOVE_REQUEST) {
			/* If the sender's up bandwidth is more than this node, send move grant message
			* along with its source-to-end delay. Also forward the request to its neighbors.*/
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
			/* Buffer all the move grant messages and choose the node with minimum source-to-end delay
			*  to replace when the moveAcceptTimer is triggered.*/
			if (simpleMeshmsg->hasPar("sourceToEndDelay")) {
				neighborSEDBuffer[simpleMeshmsg->getSrcNode()]=simpleMeshmsg->par("sourceToEndDelay").doubleValue();
			}
		}
		else if (simpleMeshmsg->getCommand() == MOVE_ACCEPT) {
			// The node needs to be replaced by the sender. Sends its parents' information in ack.
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
		else if (simpleMeshmsg->getCommand() == PARENT_REQUEST && LV->neighbourDownBandwidthReceived.size() > 0) {
			// Sends the parents' address to the node whose parent has left.
			SimpleMeshParentListMessage* parentList = new SimpleMeshParentListMessage("parentList");
			parentList->setCommand(PARENT_LIST);
			parentList->setSrcNode(thisNode);
			parentList->setBitLength(SIMPLEMESHMESSAGE_L(msg));
			parentList->setParentsArraySize(LV->neighbourDownBandwidthReceived.size());

			std::map <TransportAddress,double>::iterator it;
			unsigned int i=0;
			for (it=LV->neighbourDownBandwidthReceived.begin(); it != LV->neighbourDownBandwidthReceived.end(); ++it, i++) {
				parentList->setParents(i,it->first);
			}
			sendMessageToUDP(simpleMeshmsg->getSrcNode(),parentList);
		}
		delete simpleMeshmsg;
	}
	else if (dynamic_cast<SimpleMeshParentListMessage*>(msg) != NULL) {
		SimpleMeshParentListMessage* simpleMeshParentListmsg = check_and_cast<SimpleMeshParentListMessage*>(msg);

		if (simpleMeshParentListmsg->getCommand() == ACCEPT_ACK && simpleMeshParentListmsg->getParentsArraySize()>0) {
			/* The node must replace the sender. Sends replace message to its parents received in this message,
			* asking the parents to replace sender with this node as child.*/
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

			cancelEvent(parentReplaceTimer);
			scheduleAt(simTime()+2,parentReplaceTimer);
			delete replaceNode;
		}

		else if (simpleMeshParentListmsg->getCommand() == PARENT_LIST && LV->getCurrentDownBandwidth()<videoAverageRate*1024) {
			// The node sends join request to the nodes received in this message.
			SimpleMeshMessage* joinRequest = new SimpleMeshMessage("joinRequest");
			joinRequest->setCommand(JOIN_REQUEST);
			joinRequest->setSrcNode(thisNode);
			joinRequest->setBitLength(SIMPLEMESHMESSAGE_L(msg));
			int limit = 0;

			if(simpleMeshParentListmsg->getParentsArraySize() < activeNeighbors - LV->neighbourDownBandwidthReceived.size() )
				limit = simpleMeshParentListmsg->getParentsArraySize();
			else
				limit = activeNeighbors - LV->neighbourDownBandwidthReceived.size();

			numOfPowerResponsesExpected+=limit;
			for(unsigned int i=0 ; i < simpleMeshParentListmsg->getParentsArraySize() ; i++) {
				if (LV->neighbourDownBandwidthReceived.find(simpleMeshParentListmsg->getParents(i)) == LV->neighbourDownBandwidthReceived.end() && limit-- > 0)
					sendMessageToUDP(simpleMeshParentListmsg->getParents(i),joinRequest->dup());
			}

			cancelEvent(neighborSelectionTimer);
			scheduleAt(simTime()+1,neighborSelectionTimer);

			delete joinRequest;
		}

		delete simpleMeshParentListmsg;
	}
	else if (dynamic_cast<SimpleMeshReplaceMessage*>(msg) != NULL) {
		SimpleMeshReplaceMessage* simpleMeshReplacemsg = check_and_cast<SimpleMeshReplaceMessage*>(msg);

		if (simpleMeshReplacemsg->getCommand() == REPLACE) {
			/* The node replaces its child 'nodeToBeReplaced' with sender.
			* Disconnects from current child and sends ack to sender.*/
			TransportAddress nodeToBeReplaced = simpleMeshReplacemsg->getNodeToBeReplaced();
			if (LV->neighbourUpBandwidthAllotment.find(nodeToBeReplaced) != LV->neighbourUpBandwidthAllotment.end()) {
				LV->neighbourUpBandwidthAllotment[simpleMeshReplacemsg->getSrcNode()]=LV->neighbourUpBandwidthAllotment[nodeToBeReplaced];
				LV->neighbourUpBandwidthAllotment.erase(nodeToBeReplaced);

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

	// Send a disconnect message to all the neighbors before leaving.
	SimpleMeshMessage* disconnectMsg = new SimpleMeshMessage("disconnect");
	disconnectMsg->setCommand(DISCONNECT);
	disconnectMsg->setSrcNode(thisNode);
	disconnectMsg->setBitLength(SIMPLEMESHMESSAGE_L(msg));

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
		stat_disconnectMessages += 1;
		stat_disconnectMessagesBytesSent += disconnectMsg->getByteLength();
	}
	VideoMessage* videoMsg = new VideoMessage();
	videoMsg->setCommand(LEAVING);
	send(videoMsg,"appOut");
	delete disconnectMsg;
}

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
	// Remove the leaving neighbor from all the lists.
	deleteOverlayNeighborArrow(Node);

	if (LV->neighbourUpBandwidthAllotment.find(Node)!=LV->neighbourUpBandwidthAllotment.end()) {
		LV->neighbourUpBandwidthAllotment.erase(Node);
	}
	if(!isSource)
	{
		if (LV->neighbourDownBandwidthReceived.find(Node)!=LV->neighbourDownBandwidthReceived.end()) {
			LV->neighbourDelay.erase(Node);
			LV->neighbourDownBandwidthReceived.erase(Node);
			if (!parentLeft) {
				parentLeft = true;
				parentLeftTime = simTime().dbl();
			}
			countParentLeft++;

			if (LV->neighbourDownBandwidthReceived.empty()) {
				cancelEvent(meshJoinRequestTimer);
				scheduleAt(simTime(),meshJoinRequestTimer);
			}
			else {
				// If the leaving node was a parent, request new parent from existing parents.
				SimpleMeshMessage* parentRequestMsg = new SimpleMeshMessage("parentRequest");
				parentRequestMsg->setCommand(PARENT_REQUEST);
				parentRequestMsg->setSrcNode(thisNode);
				parentRequestMsg->setBitLength(SIMPLEMESHMESSAGE_L(msg));

				std::map <TransportAddress,double>::iterator it;
				for (it=LV->neighbourDownBandwidthReceived.begin(); it != LV->neighbourDownBandwidthReceived.end(); it++)
					sendMessageToUDP(it->first,parentRequestMsg->dup());

				delete parentRequestMsg;
			}
		}
	}
	VideoMessage* videoMsg = new VideoMessage();
	videoMsg->setCommand(NEIGHBOR_LEAVE);
	videoMsg->setSrcNode(Node);
	send(videoMsg,"appOut");
}
void SimpleMesh::finishOverlay()
{
	cancelAndDelete(checkNeighborTimer);
	cancelAndDelete(aliveNotificationTimer);
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

	if (!isSource) {
		// Stat Collection for Startup Delay : Peer Selection Time and Buffering time
		if (stat_peerSelectionTime > 0) {
			globalStatistics->addStdDev("SimpleMesh: Peer Selection Time", stat_peerSelectionTime);

			if (firstChunkTime > 0) {
				stat_peerSelectionToFirstChunkTime = firstChunkTime - peerSelectionTime;
				globalStatistics->addStdDev("SimpleMesh: Peer Selection to First Chunk Received Time", stat_peerSelectionToFirstChunkTime);
			}
		}

		// Stat Collection for Parent Reselection
		if (countParentLeft > 0 && sum_ParentReselectionTime>0) {
			stat_parentReselectionTime = sum_ParentReselectionTime/countParentLeft;
			std::stringstream buf;
			buf << "SimpleMesh: Average Parent Reselection time for parents leaving " << countParentLeft;
			std::string s = buf.str();
			globalStatistics->addStdDev(s.c_str(), stat_parentReselectionTime);
			globalStatistics->addStdDev("SimpleMesh: Parent Reselection Time", stat_parentReselectionTime);
		}
	}

	setOverlayReady(false);
}
