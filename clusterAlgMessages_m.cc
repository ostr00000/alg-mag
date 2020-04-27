//
// Generated file, do not edit! Created by nedtool 5.6 from inet/routing/cluster_alg/clusterAlgMessages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include "clusterAlgMessages_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace {
template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)(static_cast<const omnetpp::cObject *>(t));
}

template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && !std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)dynamic_cast<const void *>(t);
}

template <class T> inline
typename std::enable_if<!std::is_polymorphic<T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)static_cast<const void *>(t);
}

}

namespace inet {

// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule to generate operator<< for shared_ptr<T>
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const std::shared_ptr<T>& t) { return out << t.get(); }

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("inet::MessageType");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("inet::MessageType"));
    e->insert(HELLO, "HELLO");
    e->insert(TOPOLOGY_CONTROL, "TOPOLOGY_CONTROL");
)

Register_Class(ClusterAlgBase)

ClusterAlgBase::ClusterAlgBase() : ::inet::FieldsChunk()
{
}

ClusterAlgBase::ClusterAlgBase(const ClusterAlgBase& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

ClusterAlgBase::~ClusterAlgBase()
{
}

ClusterAlgBase& ClusterAlgBase::operator=(const ClusterAlgBase& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void ClusterAlgBase::copy(const ClusterAlgBase& other)
{
    this->messageType = other.messageType;
    this->srcAddress = other.srcAddress;
    this->nextAddress = other.nextAddress;
    this->sequencenumber = other.sequencenumber;
    this->hopdistance = other.hopdistance;
    this->srcId = other.srcId;
}

void ClusterAlgBase::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->messageType);
    doParsimPacking(b,this->srcAddress);
    doParsimPacking(b,this->nextAddress);
    doParsimPacking(b,this->sequencenumber);
    doParsimPacking(b,this->hopdistance);
    doParsimPacking(b,this->srcId);
}

void ClusterAlgBase::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->messageType);
    doParsimUnpacking(b,this->srcAddress);
    doParsimUnpacking(b,this->nextAddress);
    doParsimUnpacking(b,this->sequencenumber);
    doParsimUnpacking(b,this->hopdistance);
    doParsimUnpacking(b,this->srcId);
}

inet::MessageType ClusterAlgBase::getMessageType() const
{
    return this->messageType;
}

void ClusterAlgBase::setMessageType(inet::MessageType messageType)
{
    handleChange();
    this->messageType = messageType;
}

const Ipv4Address& ClusterAlgBase::getSrcAddress() const
{
    return this->srcAddress;
}

void ClusterAlgBase::setSrcAddress(const Ipv4Address& srcAddress)
{
    handleChange();
    this->srcAddress = srcAddress;
}

const Ipv4Address& ClusterAlgBase::getNextAddress() const
{
    return this->nextAddress;
}

void ClusterAlgBase::setNextAddress(const Ipv4Address& nextAddress)
{
    handleChange();
    this->nextAddress = nextAddress;
}

unsigned int ClusterAlgBase::getSequencenumber() const
{
    return this->sequencenumber;
}

void ClusterAlgBase::setSequencenumber(unsigned int sequencenumber)
{
    handleChange();
    this->sequencenumber = sequencenumber;
}

int ClusterAlgBase::getHopdistance() const
{
    return this->hopdistance;
}

void ClusterAlgBase::setHopdistance(int hopdistance)
{
    handleChange();
    this->hopdistance = hopdistance;
}

const Ipv4Address& ClusterAlgBase::getSrcId() const
{
    return this->srcId;
}

void ClusterAlgBase::setSrcId(const Ipv4Address& srcId)
{
    handleChange();
    this->srcId = srcId;
}

class ClusterAlgBaseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_messageType,
        FIELD_srcAddress,
        FIELD_nextAddress,
        FIELD_sequencenumber,
        FIELD_hopdistance,
        FIELD_srcId,
    };
  public:
    ClusterAlgBaseDescriptor();
    virtual ~ClusterAlgBaseDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(ClusterAlgBaseDescriptor)

ClusterAlgBaseDescriptor::ClusterAlgBaseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::ClusterAlgBase)), "inet::FieldsChunk")
{
    propertynames = nullptr;
}

ClusterAlgBaseDescriptor::~ClusterAlgBaseDescriptor()
{
    delete[] propertynames;
}

bool ClusterAlgBaseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ClusterAlgBase *>(obj)!=nullptr;
}

const char **ClusterAlgBaseDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ClusterAlgBaseDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ClusterAlgBaseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount() : 6;
}

unsigned int ClusterAlgBaseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_messageType
        0,    // FIELD_srcAddress
        0,    // FIELD_nextAddress
        FD_ISEDITABLE,    // FIELD_sequencenumber
        FD_ISEDITABLE,    // FIELD_hopdistance
        0,    // FIELD_srcId
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *ClusterAlgBaseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "messageType",
        "srcAddress",
        "nextAddress",
        "sequencenumber",
        "hopdistance",
        "srcId",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int ClusterAlgBaseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'm' && strcmp(fieldName, "messageType") == 0) return base+0;
    if (fieldName[0] == 's' && strcmp(fieldName, "srcAddress") == 0) return base+1;
    if (fieldName[0] == 'n' && strcmp(fieldName, "nextAddress") == 0) return base+2;
    if (fieldName[0] == 's' && strcmp(fieldName, "sequencenumber") == 0) return base+3;
    if (fieldName[0] == 'h' && strcmp(fieldName, "hopdistance") == 0) return base+4;
    if (fieldName[0] == 's' && strcmp(fieldName, "srcId") == 0) return base+5;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ClusterAlgBaseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "inet::MessageType",    // FIELD_messageType
        "inet::Ipv4Address",    // FIELD_srcAddress
        "inet::Ipv4Address",    // FIELD_nextAddress
        "unsigned int",    // FIELD_sequencenumber
        "int",    // FIELD_hopdistance
        "inet::Ipv4Address",    // FIELD_srcId
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **ClusterAlgBaseDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_messageType: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *ClusterAlgBaseDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_messageType:
            if (!strcmp(propertyname, "enum")) return "inet::MessageType";
            return nullptr;
        default: return nullptr;
    }
}

int ClusterAlgBaseDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgBase *pp = (ClusterAlgBase *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ClusterAlgBaseDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgBase *pp = (ClusterAlgBase *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ClusterAlgBaseDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgBase *pp = (ClusterAlgBase *)object; (void)pp;
    switch (field) {
        case FIELD_messageType: return enum2string(pp->getMessageType(), "inet::MessageType");
        case FIELD_srcAddress: return pp->getSrcAddress().str();
        case FIELD_nextAddress: return pp->getNextAddress().str();
        case FIELD_sequencenumber: return ulong2string(pp->getSequencenumber());
        case FIELD_hopdistance: return long2string(pp->getHopdistance());
        case FIELD_srcId: return pp->getSrcId().str();
        default: return "";
    }
}

bool ClusterAlgBaseDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgBase *pp = (ClusterAlgBase *)object; (void)pp;
    switch (field) {
        case FIELD_messageType: pp->setMessageType((inet::MessageType)string2enum(value, "inet::MessageType")); return true;
        case FIELD_sequencenumber: pp->setSequencenumber(string2ulong(value)); return true;
        case FIELD_hopdistance: pp->setHopdistance(string2long(value)); return true;
        default: return false;
    }
}

const char *ClusterAlgBaseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *ClusterAlgBaseDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgBase *pp = (ClusterAlgBase *)object; (void)pp;
    switch (field) {
        case FIELD_srcAddress: return toVoidPtr(&pp->getSrcAddress()); break;
        case FIELD_nextAddress: return toVoidPtr(&pp->getNextAddress()); break;
        case FIELD_srcId: return toVoidPtr(&pp->getSrcId()); break;
        default: return nullptr;
    }
}

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("inet::NodeState");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("inet::NodeState"));
    e->insert(UNDECIDED, "UNDECIDED");
    e->insert(LIDER, "LIDER");
    e->insert(MEMBER, "MEMBER");
)

Register_Class(ClusterAlgHello)

ClusterAlgHello::ClusterAlgHello() : ::inet::ClusterAlgBase()
{
}

ClusterAlgHello::ClusterAlgHello(const ClusterAlgHello& other) : ::inet::ClusterAlgBase(other)
{
    copy(other);
}

ClusterAlgHello::~ClusterAlgHello()
{
    delete [] this->neighbors;
}

ClusterAlgHello& ClusterAlgHello::operator=(const ClusterAlgHello& other)
{
    if (this == &other) return *this;
    ::inet::ClusterAlgBase::operator=(other);
    copy(other);
    return *this;
}

void ClusterAlgHello::copy(const ClusterAlgHello& other)
{
    this->neighborsNum = other.neighborsNum;
    this->undecidedNeighborsNum = other.undecidedNeighborsNum;
    this->state = other.state;
    this->clusterHeadId = other.clusterHeadId;
    delete [] this->neighbors;
    this->neighbors = (other.neighbors_arraysize==0) ? nullptr : new Ipv4Address[other.neighbors_arraysize];
    neighbors_arraysize = other.neighbors_arraysize;
    for (size_t i = 0; i < neighbors_arraysize; i++) {
        this->neighbors[i] = other.neighbors[i];
    }
}

void ClusterAlgHello::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::ClusterAlgBase::parsimPack(b);
    doParsimPacking(b,this->neighborsNum);
    doParsimPacking(b,this->undecidedNeighborsNum);
    doParsimPacking(b,this->state);
    doParsimPacking(b,this->clusterHeadId);
    b->pack(neighbors_arraysize);
    doParsimArrayPacking(b,this->neighbors,neighbors_arraysize);
}

void ClusterAlgHello::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::ClusterAlgBase::parsimUnpack(b);
    doParsimUnpacking(b,this->neighborsNum);
    doParsimUnpacking(b,this->undecidedNeighborsNum);
    doParsimUnpacking(b,this->state);
    doParsimUnpacking(b,this->clusterHeadId);
    delete [] this->neighbors;
    b->unpack(neighbors_arraysize);
    if (neighbors_arraysize == 0) {
        this->neighbors = nullptr;
    } else {
        this->neighbors = new Ipv4Address[neighbors_arraysize];
        doParsimArrayUnpacking(b,this->neighbors,neighbors_arraysize);
    }
}

int ClusterAlgHello::getNeighborsNum() const
{
    return this->neighborsNum;
}

void ClusterAlgHello::setNeighborsNum(int neighborsNum)
{
    handleChange();
    this->neighborsNum = neighborsNum;
}

int ClusterAlgHello::getUndecidedNeighborsNum() const
{
    return this->undecidedNeighborsNum;
}

void ClusterAlgHello::setUndecidedNeighborsNum(int undecidedNeighborsNum)
{
    handleChange();
    this->undecidedNeighborsNum = undecidedNeighborsNum;
}

inet::NodeState ClusterAlgHello::getState() const
{
    return this->state;
}

void ClusterAlgHello::setState(inet::NodeState state)
{
    handleChange();
    this->state = state;
}

const Ipv4Address& ClusterAlgHello::getClusterHeadId() const
{
    return this->clusterHeadId;
}

void ClusterAlgHello::setClusterHeadId(const Ipv4Address& clusterHeadId)
{
    handleChange();
    this->clusterHeadId = clusterHeadId;
}

size_t ClusterAlgHello::getNeighborsArraySize() const
{
    return neighbors_arraysize;
}

const Ipv4Address& ClusterAlgHello::getNeighbors(size_t k) const
{
    if (k >= neighbors_arraysize) throw omnetpp::cRuntimeError("Array of size neighbors_arraysize indexed by %lu", (unsigned long)k);
    return this->neighbors[k];
}

void ClusterAlgHello::setNeighborsArraySize(size_t newSize)
{
    handleChange();
    Ipv4Address *neighbors2 = (newSize==0) ? nullptr : new Ipv4Address[newSize];
    size_t minSize = neighbors_arraysize < newSize ? neighbors_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        neighbors2[i] = this->neighbors[i];
    delete [] this->neighbors;
    this->neighbors = neighbors2;
    neighbors_arraysize = newSize;
}

void ClusterAlgHello::setNeighbors(size_t k, const Ipv4Address& neighbors)
{
    if (k >= neighbors_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    this->neighbors[k] = neighbors;
}

void ClusterAlgHello::insertNeighbors(size_t k, const Ipv4Address& neighbors)
{
    handleChange();
    if (k > neighbors_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = neighbors_arraysize + 1;
    Ipv4Address *neighbors2 = new Ipv4Address[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        neighbors2[i] = this->neighbors[i];
    neighbors2[k] = neighbors;
    for (i = k + 1; i < newSize; i++)
        neighbors2[i] = this->neighbors[i-1];
    delete [] this->neighbors;
    this->neighbors = neighbors2;
    neighbors_arraysize = newSize;
}

void ClusterAlgHello::insertNeighbors(const Ipv4Address& neighbors)
{
    insertNeighbors(neighbors_arraysize, neighbors);
}

void ClusterAlgHello::eraseNeighbors(size_t k)
{
    if (k >= neighbors_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    size_t newSize = neighbors_arraysize - 1;
    Ipv4Address *neighbors2 = (newSize == 0) ? nullptr : new Ipv4Address[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        neighbors2[i] = this->neighbors[i];
    for (i = k; i < newSize; i++)
        neighbors2[i] = this->neighbors[i+1];
    delete [] this->neighbors;
    this->neighbors = neighbors2;
    neighbors_arraysize = newSize;
}

class ClusterAlgHelloDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_neighborsNum,
        FIELD_undecidedNeighborsNum,
        FIELD_state,
        FIELD_clusterHeadId,
        FIELD_neighbors,
    };
  public:
    ClusterAlgHelloDescriptor();
    virtual ~ClusterAlgHelloDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(ClusterAlgHelloDescriptor)

ClusterAlgHelloDescriptor::ClusterAlgHelloDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::ClusterAlgHello)), "inet::ClusterAlgBase")
{
    propertynames = nullptr;
}

ClusterAlgHelloDescriptor::~ClusterAlgHelloDescriptor()
{
    delete[] propertynames;
}

bool ClusterAlgHelloDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ClusterAlgHello *>(obj)!=nullptr;
}

const char **ClusterAlgHelloDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ClusterAlgHelloDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ClusterAlgHelloDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int ClusterAlgHelloDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_neighborsNum
        FD_ISEDITABLE,    // FIELD_undecidedNeighborsNum
        FD_ISEDITABLE,    // FIELD_state
        0,    // FIELD_clusterHeadId
        FD_ISARRAY,    // FIELD_neighbors
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *ClusterAlgHelloDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "neighborsNum",
        "undecidedNeighborsNum",
        "state",
        "clusterHeadId",
        "neighbors",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int ClusterAlgHelloDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'n' && strcmp(fieldName, "neighborsNum") == 0) return base+0;
    if (fieldName[0] == 'u' && strcmp(fieldName, "undecidedNeighborsNum") == 0) return base+1;
    if (fieldName[0] == 's' && strcmp(fieldName, "state") == 0) return base+2;
    if (fieldName[0] == 'c' && strcmp(fieldName, "clusterHeadId") == 0) return base+3;
    if (fieldName[0] == 'n' && strcmp(fieldName, "neighbors") == 0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ClusterAlgHelloDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_neighborsNum
        "int",    // FIELD_undecidedNeighborsNum
        "inet::NodeState",    // FIELD_state
        "inet::Ipv4Address",    // FIELD_clusterHeadId
        "inet::Ipv4Address",    // FIELD_neighbors
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **ClusterAlgHelloDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_state: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *ClusterAlgHelloDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_state:
            if (!strcmp(propertyname, "enum")) return "inet::NodeState";
            return nullptr;
        default: return nullptr;
    }
}

int ClusterAlgHelloDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgHello *pp = (ClusterAlgHello *)object; (void)pp;
    switch (field) {
        case FIELD_neighbors: return pp->getNeighborsArraySize();
        default: return 0;
    }
}

const char *ClusterAlgHelloDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgHello *pp = (ClusterAlgHello *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ClusterAlgHelloDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgHello *pp = (ClusterAlgHello *)object; (void)pp;
    switch (field) {
        case FIELD_neighborsNum: return long2string(pp->getNeighborsNum());
        case FIELD_undecidedNeighborsNum: return long2string(pp->getUndecidedNeighborsNum());
        case FIELD_state: return enum2string(pp->getState(), "inet::NodeState");
        case FIELD_clusterHeadId: return pp->getClusterHeadId().str();
        case FIELD_neighbors: return pp->getNeighbors(i).str();
        default: return "";
    }
}

bool ClusterAlgHelloDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgHello *pp = (ClusterAlgHello *)object; (void)pp;
    switch (field) {
        case FIELD_neighborsNum: pp->setNeighborsNum(string2long(value)); return true;
        case FIELD_undecidedNeighborsNum: pp->setUndecidedNeighborsNum(string2long(value)); return true;
        case FIELD_state: pp->setState((inet::NodeState)string2enum(value, "inet::NodeState")); return true;
        default: return false;
    }
}

const char *ClusterAlgHelloDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *ClusterAlgHelloDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgHello *pp = (ClusterAlgHello *)object; (void)pp;
    switch (field) {
        case FIELD_clusterHeadId: return toVoidPtr(&pp->getClusterHeadId()); break;
        case FIELD_neighbors: return toVoidPtr(&pp->getNeighbors(i)); break;
        default: return nullptr;
    }
}

Register_Class(ClusterAlgTopologyControl)

ClusterAlgTopologyControl::ClusterAlgTopologyControl() : ::inet::ClusterAlgBase()
{
}

ClusterAlgTopologyControl::ClusterAlgTopologyControl(const ClusterAlgTopologyControl& other) : ::inet::ClusterAlgBase(other)
{
    copy(other);
}

ClusterAlgTopologyControl::~ClusterAlgTopologyControl()
{
}

ClusterAlgTopologyControl& ClusterAlgTopologyControl::operator=(const ClusterAlgTopologyControl& other)
{
    if (this == &other) return *this;
    ::inet::ClusterAlgBase::operator=(other);
    copy(other);
    return *this;
}

void ClusterAlgTopologyControl::copy(const ClusterAlgTopologyControl& other)
{
}

void ClusterAlgTopologyControl::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::ClusterAlgBase::parsimPack(b);
}

void ClusterAlgTopologyControl::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::ClusterAlgBase::parsimUnpack(b);
}

class ClusterAlgTopologyControlDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ClusterAlgTopologyControlDescriptor();
    virtual ~ClusterAlgTopologyControlDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(ClusterAlgTopologyControlDescriptor)

ClusterAlgTopologyControlDescriptor::ClusterAlgTopologyControlDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::ClusterAlgTopologyControl)), "inet::ClusterAlgBase")
{
    propertynames = nullptr;
}

ClusterAlgTopologyControlDescriptor::~ClusterAlgTopologyControlDescriptor()
{
    delete[] propertynames;
}

bool ClusterAlgTopologyControlDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ClusterAlgTopologyControl *>(obj)!=nullptr;
}

const char **ClusterAlgTopologyControlDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ClusterAlgTopologyControlDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ClusterAlgTopologyControlDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ClusterAlgTopologyControlDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ClusterAlgTopologyControlDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ClusterAlgTopologyControlDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ClusterAlgTopologyControlDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ClusterAlgTopologyControlDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *ClusterAlgTopologyControlDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int ClusterAlgTopologyControlDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgTopologyControl *pp = (ClusterAlgTopologyControl *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ClusterAlgTopologyControlDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgTopologyControl *pp = (ClusterAlgTopologyControl *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ClusterAlgTopologyControlDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgTopologyControl *pp = (ClusterAlgTopologyControl *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ClusterAlgTopologyControlDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgTopologyControl *pp = (ClusterAlgTopologyControl *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ClusterAlgTopologyControlDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ClusterAlgTopologyControlDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ClusterAlgTopologyControl *pp = (ClusterAlgTopologyControl *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

} // namespace inet

