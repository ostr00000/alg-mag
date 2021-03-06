//
// Generated file, do not edit! Created by nedtool 5.6 from inet/routing/cluster_alg/clusterAlgMessages.msg.
//

#ifndef __INET_CLUSTERALGMESSAGES_M_H
#define __INET_CLUSTERALGMESSAGES_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif

// dll export symbol
#ifndef INET_API
#  if defined(INET_EXPORT)
#    define INET_API  OPP_DLLEXPORT
#  elif defined(INET_IMPORT)
#    define INET_API  OPP_DLLIMPORT
#  else
#    define INET_API
#  endif
#endif


namespace inet {

class ClusterAlgBase;
class ClusterAlgHello;
class ClusterAlgTopologyControl;
} // namespace inet

#include "inet/common/INETDefs_m.h" // import inet.common.INETDefs

#include "inet/common/packet/chunk/Chunk_m.h" // import inet.common.packet.chunk.Chunk

#include "inet/networklayer/contract/ipv4/Ipv4Address_m.h" // import inet.networklayer.contract.ipv4.Ipv4Address


namespace inet {

/**
 * Enum generated from <tt>inet/routing/cluster_alg/clusterAlgMessages.msg:8</tt> by nedtool.
 * <pre>
 * enum MessageType
 * {
 *     HELLO = 1;
 *     TOPOLOGY_CONTROL = 2;
 * }
 * </pre>
 */
enum MessageType {
    HELLO = 1,
    TOPOLOGY_CONTROL = 2
};

/**
 * Class generated from <tt>inet/routing/cluster_alg/clusterAlgMessages.msg:14</tt> by nedtool.
 * <pre>
 * class ClusterAlgBase extends FieldsChunk
 * {
 *     MessageType messageType;
 *     unsigned int sequencenumber;
 *     int hopdistance;
 *     Ipv4Address srcId;
 * }
 * </pre>
 */
class INET_API ClusterAlgBase : public ::inet::FieldsChunk
{
  protected:
    inet::MessageType messageType = static_cast<inet::MessageType>(-1);
    unsigned int sequencenumber = 0;
    int hopdistance = 0;
    Ipv4Address srcId;

  private:
    void copy(const ClusterAlgBase& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ClusterAlgBase&);

  public:
    ClusterAlgBase();
    ClusterAlgBase(const ClusterAlgBase& other);
    virtual ~ClusterAlgBase();
    ClusterAlgBase& operator=(const ClusterAlgBase& other);
    virtual ClusterAlgBase *dup() const override {return new ClusterAlgBase(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual inet::MessageType getMessageType() const;
    virtual void setMessageType(inet::MessageType messageType);
    virtual unsigned int getSequencenumber() const;
    virtual void setSequencenumber(unsigned int sequencenumber);
    virtual int getHopdistance() const;
    virtual void setHopdistance(int hopdistance);
    virtual const Ipv4Address& getSrcId() const;
    virtual Ipv4Address& getSrcIdForUpdate() { handleChange();return const_cast<Ipv4Address&>(const_cast<ClusterAlgBase*>(this)->getSrcId());}
    virtual void setSrcId(const Ipv4Address& srcId);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ClusterAlgBase& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ClusterAlgBase& obj) {obj.parsimUnpack(b);}

/**
 * Enum generated from <tt>inet/routing/cluster_alg/clusterAlgMessages.msg:22</tt> by nedtool.
 * <pre>
 * enum NodeState
 * {
 *     UNDECIDED = 1;
 *     LEADER = 2;
 *     MEMBER = 3;
 * }
 * </pre>
 */
enum NodeState {
    UNDECIDED = 1,
    LEADER = 2,
    MEMBER = 3
};

/**
 * Class generated from <tt>inet/routing/cluster_alg/clusterAlgMessages.msg:28</tt> by nedtool.
 * <pre>
 * class ClusterAlgHello extends ClusterAlgBase
 * {
 *     int neighborsNum;
 *     int undecidedNeighborsNum;
 *     NodeState state;
 *     Ipv4Address clusterHeadId;
 *     Ipv4Address neighbors[];
 *     Ipv4Address neighborsCluster[];
 * }
 * </pre>
 */
class INET_API ClusterAlgHello : public ::inet::ClusterAlgBase
{
  protected:
    int neighborsNum = 0;
    int undecidedNeighborsNum = 0;
    inet::NodeState state = static_cast<inet::NodeState>(-1);
    Ipv4Address clusterHeadId;
    Ipv4Address *neighbors = nullptr;
    size_t neighbors_arraysize = 0;
    Ipv4Address *neighborsCluster = nullptr;
    size_t neighborsCluster_arraysize = 0;

  private:
    void copy(const ClusterAlgHello& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ClusterAlgHello&);

  public:
    ClusterAlgHello();
    ClusterAlgHello(const ClusterAlgHello& other);
    virtual ~ClusterAlgHello();
    ClusterAlgHello& operator=(const ClusterAlgHello& other);
    virtual ClusterAlgHello *dup() const override {return new ClusterAlgHello(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getNeighborsNum() const;
    virtual void setNeighborsNum(int neighborsNum);
    virtual int getUndecidedNeighborsNum() const;
    virtual void setUndecidedNeighborsNum(int undecidedNeighborsNum);
    virtual inet::NodeState getState() const;
    virtual void setState(inet::NodeState state);
    virtual const Ipv4Address& getClusterHeadId() const;
    virtual Ipv4Address& getClusterHeadIdForUpdate() { handleChange();return const_cast<Ipv4Address&>(const_cast<ClusterAlgHello*>(this)->getClusterHeadId());}
    virtual void setClusterHeadId(const Ipv4Address& clusterHeadId);
    virtual void setNeighborsArraySize(size_t size);
    virtual size_t getNeighborsArraySize() const;
    virtual const Ipv4Address& getNeighbors(size_t k) const;
    virtual Ipv4Address& getNeighborsForUpdate(size_t k) { handleChange();return const_cast<Ipv4Address&>(const_cast<ClusterAlgHello*>(this)->getNeighbors(k));}
    virtual void setNeighbors(size_t k, const Ipv4Address& neighbors);
    virtual void insertNeighbors(const Ipv4Address& neighbors);
    virtual void insertNeighbors(size_t k, const Ipv4Address& neighbors);
    virtual void eraseNeighbors(size_t k);
    virtual void setNeighborsClusterArraySize(size_t size);
    virtual size_t getNeighborsClusterArraySize() const;
    virtual const Ipv4Address& getNeighborsCluster(size_t k) const;
    virtual Ipv4Address& getNeighborsClusterForUpdate(size_t k) { handleChange();return const_cast<Ipv4Address&>(const_cast<ClusterAlgHello*>(this)->getNeighborsCluster(k));}
    virtual void setNeighborsCluster(size_t k, const Ipv4Address& neighborsCluster);
    virtual void insertNeighborsCluster(const Ipv4Address& neighborsCluster);
    virtual void insertNeighborsCluster(size_t k, const Ipv4Address& neighborsCluster);
    virtual void eraseNeighborsCluster(size_t k);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ClusterAlgHello& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ClusterAlgHello& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>inet/routing/cluster_alg/clusterAlgMessages.msg:37</tt> by nedtool.
 * <pre>
 * class ClusterAlgTopologyControl extends ClusterAlgBase
 * {
 *     Ipv4Address allowedToForward[];
 *     Ipv4Address members[];
 *     Ipv4Address neighborsCluster[];
 * }
 * </pre>
 */
class INET_API ClusterAlgTopologyControl : public ::inet::ClusterAlgBase
{
  protected:
    Ipv4Address *allowedToForward = nullptr;
    size_t allowedToForward_arraysize = 0;
    Ipv4Address *members = nullptr;
    size_t members_arraysize = 0;
    Ipv4Address *neighborsCluster = nullptr;
    size_t neighborsCluster_arraysize = 0;

  private:
    void copy(const ClusterAlgTopologyControl& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const ClusterAlgTopologyControl&);

  public:
    ClusterAlgTopologyControl();
    ClusterAlgTopologyControl(const ClusterAlgTopologyControl& other);
    virtual ~ClusterAlgTopologyControl();
    ClusterAlgTopologyControl& operator=(const ClusterAlgTopologyControl& other);
    virtual ClusterAlgTopologyControl *dup() const override {return new ClusterAlgTopologyControl(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual void setAllowedToForwardArraySize(size_t size);
    virtual size_t getAllowedToForwardArraySize() const;
    virtual const Ipv4Address& getAllowedToForward(size_t k) const;
    virtual Ipv4Address& getAllowedToForwardForUpdate(size_t k) { handleChange();return const_cast<Ipv4Address&>(const_cast<ClusterAlgTopologyControl*>(this)->getAllowedToForward(k));}
    virtual void setAllowedToForward(size_t k, const Ipv4Address& allowedToForward);
    virtual void insertAllowedToForward(const Ipv4Address& allowedToForward);
    virtual void insertAllowedToForward(size_t k, const Ipv4Address& allowedToForward);
    virtual void eraseAllowedToForward(size_t k);
    virtual void setMembersArraySize(size_t size);
    virtual size_t getMembersArraySize() const;
    virtual const Ipv4Address& getMembers(size_t k) const;
    virtual Ipv4Address& getMembersForUpdate(size_t k) { handleChange();return const_cast<Ipv4Address&>(const_cast<ClusterAlgTopologyControl*>(this)->getMembers(k));}
    virtual void setMembers(size_t k, const Ipv4Address& members);
    virtual void insertMembers(const Ipv4Address& members);
    virtual void insertMembers(size_t k, const Ipv4Address& members);
    virtual void eraseMembers(size_t k);
    virtual void setNeighborsClusterArraySize(size_t size);
    virtual size_t getNeighborsClusterArraySize() const;
    virtual const Ipv4Address& getNeighborsCluster(size_t k) const;
    virtual Ipv4Address& getNeighborsClusterForUpdate(size_t k) { handleChange();return const_cast<Ipv4Address&>(const_cast<ClusterAlgTopologyControl*>(this)->getNeighborsCluster(k));}
    virtual void setNeighborsCluster(size_t k, const Ipv4Address& neighborsCluster);
    virtual void insertNeighborsCluster(const Ipv4Address& neighborsCluster);
    virtual void insertNeighborsCluster(size_t k, const Ipv4Address& neighborsCluster);
    virtual void eraseNeighborsCluster(size_t k);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ClusterAlgTopologyControl& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ClusterAlgTopologyControl& obj) {obj.parsimUnpack(b);}

} // namespace inet

#endif // ifndef __INET_CLUSTERALGMESSAGES_M_H

