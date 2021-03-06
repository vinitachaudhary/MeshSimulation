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
 * @file SimpleMesh.h
 * @author Yasser Seyyedi, Behnam Ahmadifar
 */

// edited by vinita

#ifndef SIMPLEMESH_H_
#define SIMPLEMESH_H_

#include <omnetpp.h>
#include <OverlayKey.h>
#include <TransportAddress.h>
#include <GlobalNodeList.h>
#include "SimpleMeshMessage_m.h"
#include <DenaCastOverlay.h>
#include <queue>

struct neighborPowerMap {
	TransportAddress tAddress; 	// Transport address of the neighbor
	double sourceToEndDelay;	// Source-to-end delay of the node through this neighbor.
	double residualUpBandwidth;	// Residual Upload Bandwidth of this neighbor.
	double power;				// Power of this neighbor.
};

// Class to compare neighborPowerMap by power.
class compareNeighbourPowerMap {
public :
	bool operator()(neighborPowerMap N1, neighborPowerMap N2) {
		return N1.power<N2.power;
	}
};

class SimpleMesh : public DenaCastOverlay
{
protected:
    bool isSource;/**< whether this node is server*/
    int activeNeighbors;/**< Neighbor that node connects to them*/
    int passiveNeighbors; /**< Neighbors that node let them connect to it*/
    int neighborNum; /**< Sum of active and passive neighbors*/
    double neighborNotificationPeriod; /**< period to notify tracker about number of current neighbor*/
    double videoAverageRate; /**< average video bit rate for calculating neighbor num*/
    bool adaptiveNeighboring; /**< true if we want to have adaptive neighbors*/
    bool isRegistered; /**< if this node registered in the tracker */
    bool serverGradualNeighboring; /**< true if gradual neighbor is required for source node*/
    TransportAddress trackerAddress; /**< Transport address of tracker node */

    /*	Priority queue to hold neighbors along with their power, so as to choose neighbor with high power. */
    std::priority_queue<neighborPowerMap, std::vector<neighborPowerMap>, compareNeighbourPowerMap> neighborPowerBuffer;
    std::map <TransportAddress,double> neighborSEDBuffer;	// Buffer to store source-to-end delay of nodes which grant move request
    int numOfPowerResponsesExpected;			// Number to power requests sent by node.
    std::map <TransportAddress,double> oldParents;		// Stores the parents of a node before it moves.
    std::map <TransportAddress,double> neighborTimeOut; // Stores TimeOut of neighbors for Keep-Alive.

    /**
     * Register node in the tracker
     */
    void selfRegister();
    /**
     * Unregister node in the tracker
     */
    void selfUnRegister();
    /**
     * for leaving this method do this process for node that want to leave with notification
     * @param TransportAdress
     */
    void disconnectProcess(TransportAddress node);

    //selfMessages
    cMessage* meshJoinRequestTimer; /**< self message for scheduling neighboring*/
    cMessage* remainNotificationTimer; /**< self message for scheduling send notification to server*/
    cMessage* serverNeighborTimer; /**< for gradual neighboring this self message plan for this job */

    cMessage* neighborSelectionTimer;	// Time when the node should start selecting/rejecting neighbors by power.
    cMessage* moveRequestTimer;		// self message for sending out move Request messages.
    cMessage* moveAcceptTimer;		// self message for accepting move Grant messages.
    cMessage* parentReplaceTimer;	// self message to disconnect from all the old parents.
    cMessage* checkNeighborTimer;	// self message to check for alive neighbors and remove neighbors which left.
    cMessage* aliveNotificationTimer; // self message to send out alive messages to neighbors.

    // statistics
	uint32_t stat_TotalUpBandwidthUsage;
	uint32_t stat_TotalDownBandwidthUsage;

    uint32_t stat_joinREQ; /**< number of sent join request messages */
	uint32_t stat_joinREQBytesSent;  /**< number of sent bytes of join request messages */
	uint32_t stat_joinRSP; /**< number of sent join response messages */
	uint32_t stat_joinRSPBytesSent; /**< number of sent bytes of join response messages */
	uint32_t stat_joinACK; /**< number of sent join acknowledge messages */
	uint32_t stat_joinACKBytesSent; /**< number of sent bytes of join acknowledge messages */
	uint32_t stat_joinDNY; /**< number of sent join deny messages */
	uint32_t stat_joinDNYBytesSent; /**< number of sent bytes of join deny messages */
	uint32_t stat_disconnectMessages; /**< number of sent disconnect messages */
	uint32_t stat_disconnectMessagesBytesSent; /**< number of sent bytes of disconnect messages */
	uint32_t stat_addedNeighbors; /**< number of added neighbors during life cycle of this node */
	uint32_t stat_nummeshJoinRequestTimer; /**< number of meshJoinRequestTimer self messages */

	bool firstPeerAdded;	// whether the node has connected to peers for the first time.
	double peerJoinTime;		// Time when node is created.
	double peerSelectionTime;	// Time when peer selects its first neighbor.
	double stat_peerSelectionTime; /**< time taken by node to connect to peers at startup */
	double stat_peerSelectionToFirstChunkTime; // time taken by node after selecting a peer till it receives its first chunk.

	bool parentLeft;	// whether a parent has left recently. For calculation of Parent Reselection Time.
	double parentLeftTime;	// Time when the last parent left.
	double sum_ParentReselectionTime;	// sums the parent reselection time for each parent leave.
	double stat_parentReselectionTime;	// average time taken by node to reselect its parents.
	int countParentLeft;	// Total count of number of parents disconnecting.
public:

    /**
     * initializes base class-attributes
     *
     * @param stage the init stage
     */
	virtual void initializeOverlay(int stage);
    /**
     * Writes statistical data and removes node from bootstrap oracle
     */
	virtual void finishOverlay();
	virtual void handleTimerEvent(cMessage* msg);
	virtual void handleUDPMessage(BaseOverlayMessage* msg);
	virtual void joinOverlay();

    /**
     *notify its neighbors that it is going to leave the mesh
     */
	virtual void handleNodeGracefulLeaveNotification();
};

#endif /* SIMPLEMESH_H_ */

