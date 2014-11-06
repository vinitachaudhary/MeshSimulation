//
// Generated file, do not edit! Created by opp_msgc 4.1 from overlay/simplemesh/SimpleMeshMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "SimpleMeshMessage_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("SIMPLEMESHCommand");
    if (!e) enums.getInstance()->add(e = new cEnum("SIMPLEMESHCommand"));
    e->insert(JOIN_REQUEST, "JOIN_REQUEST");
    e->insert(JOIN_RESPONSE, "JOIN_RESPONSE");
    e->insert(JOIN_ACK, "JOIN_ACK");
    e->insert(JOIN_DENY, "JOIN_DENY");
    e->insert(DISCONNECT, "DISCONNECT");
    e->insert(ALIVE, "ALIVE");
    e->insert(MOVE_REQUEST, "MOVE_REQUEST");
    e->insert(MOVE_GRANT, "MOVE_GRANT");
    e->insert(MOVE_ACCEPT, "MOVE_ACCEPT");
    e->insert(ACCEPT_ACK, "ACCEPT_ACK");
    e->insert(REPLACE, "REPLACE");
    e->insert(REPLACE_ACK, "REPLACE_ACK");
    e->insert(DISCONNECT_PARENT, "DISCONNECT_PARENT");
    e->insert(DISCONNECT_CHILD, "DISCONNECT_CHILD");
);

Register_Class(SimpleMeshMessage);

SimpleMeshMessage::SimpleMeshMessage(const char *name, int kind) : BaseOverlayMessage(name,kind)
{
    this->command_var = 0;
}

SimpleMeshMessage::SimpleMeshMessage(const SimpleMeshMessage& other) : BaseOverlayMessage()
{
    setName(other.getName());
    operator=(other);
}

SimpleMeshMessage::~SimpleMeshMessage()
{
}

SimpleMeshMessage& SimpleMeshMessage::operator=(const SimpleMeshMessage& other)
{
    if (this==&other) return *this;
    BaseOverlayMessage::operator=(other);
    this->command_var = other.command_var;
    this->srcNode_var = other.srcNode_var;
    return *this;
}

void SimpleMeshMessage::parsimPack(cCommBuffer *b)
{
    BaseOverlayMessage::parsimPack(b);
    doPacking(b,this->command_var);
    doPacking(b,this->srcNode_var);
}

void SimpleMeshMessage::parsimUnpack(cCommBuffer *b)
{
    BaseOverlayMessage::parsimUnpack(b);
    doUnpacking(b,this->command_var);
    doUnpacking(b,this->srcNode_var);
}

int SimpleMeshMessage::getCommand() const
{
    return command_var;
}

void SimpleMeshMessage::setCommand(int command_var)
{
    this->command_var = command_var;
}

TransportAddress& SimpleMeshMessage::getSrcNode()
{
    return srcNode_var;
}

void SimpleMeshMessage::setSrcNode(const TransportAddress& srcNode_var)
{
    this->srcNode_var = srcNode_var;
}

class SimpleMeshMessageDescriptor : public cClassDescriptor
{
  public:
    SimpleMeshMessageDescriptor();
    virtual ~SimpleMeshMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(SimpleMeshMessageDescriptor);

SimpleMeshMessageDescriptor::SimpleMeshMessageDescriptor() : cClassDescriptor("SimpleMeshMessage", "BaseOverlayMessage")
{
}

SimpleMeshMessageDescriptor::~SimpleMeshMessageDescriptor()
{
}

bool SimpleMeshMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<SimpleMeshMessage *>(obj)!=NULL;
}

const char *SimpleMeshMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int SimpleMeshMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int SimpleMeshMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *SimpleMeshMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "command",
        "srcNode",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int SimpleMeshMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='c' && strcmp(fieldName, "command")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcNode")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *SimpleMeshMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "TransportAddress",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *SimpleMeshMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "SIMPLEMESHCommand";
            return NULL;
        default: return NULL;
    }
}

int SimpleMeshMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshMessage *pp = (SimpleMeshMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string SimpleMeshMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshMessage *pp = (SimpleMeshMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getCommand());
        case 1: {std::stringstream out; out << pp->getSrcNode(); return out.str();}
        default: return "";
    }
}

bool SimpleMeshMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshMessage *pp = (SimpleMeshMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setCommand(string2long(value)); return true;
        default: return false;
    }
}

const char *SimpleMeshMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        "TransportAddress",
    };
    return (field>=0 && field<2) ? fieldStructNames[field] : NULL;
}

void *SimpleMeshMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshMessage *pp = (SimpleMeshMessage *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getSrcNode()); break;
        default: return NULL;
    }
}

Register_Class(SimpleMeshParentListMessage);

SimpleMeshParentListMessage::SimpleMeshParentListMessage(const char *name, int kind) : BaseOverlayMessage(name,kind)
{
    this->command_var = 0;
    parents_arraysize = 0;
    this->parents_var = 0;
}

SimpleMeshParentListMessage::SimpleMeshParentListMessage(const SimpleMeshParentListMessage& other) : BaseOverlayMessage()
{
    setName(other.getName());
    parents_arraysize = 0;
    this->parents_var = 0;
    operator=(other);
}

SimpleMeshParentListMessage::~SimpleMeshParentListMessage()
{
    delete [] parents_var;
}

SimpleMeshParentListMessage& SimpleMeshParentListMessage::operator=(const SimpleMeshParentListMessage& other)
{
    if (this==&other) return *this;
    BaseOverlayMessage::operator=(other);
    this->command_var = other.command_var;
    this->srcNode_var = other.srcNode_var;
    delete [] this->parents_var;
    this->parents_var = (other.parents_arraysize==0) ? NULL : new ::TransportAddress[other.parents_arraysize];
    parents_arraysize = other.parents_arraysize;
    for (unsigned int i=0; i<parents_arraysize; i++)
        this->parents_var[i] = other.parents_var[i];
    return *this;
}

void SimpleMeshParentListMessage::parsimPack(cCommBuffer *b)
{
    BaseOverlayMessage::parsimPack(b);
    doPacking(b,this->command_var);
    doPacking(b,this->srcNode_var);
    b->pack(parents_arraysize);
    doPacking(b,this->parents_var,parents_arraysize);
}

void SimpleMeshParentListMessage::parsimUnpack(cCommBuffer *b)
{
    BaseOverlayMessage::parsimUnpack(b);
    doUnpacking(b,this->command_var);
    doUnpacking(b,this->srcNode_var);
    delete [] this->parents_var;
    b->unpack(parents_arraysize);
    if (parents_arraysize==0) {
        this->parents_var = 0;
    } else {
        this->parents_var = new ::TransportAddress[parents_arraysize];
        doUnpacking(b,this->parents_var,parents_arraysize);
    }
}

int SimpleMeshParentListMessage::getCommand() const
{
    return command_var;
}

void SimpleMeshParentListMessage::setCommand(int command_var)
{
    this->command_var = command_var;
}

TransportAddress& SimpleMeshParentListMessage::getSrcNode()
{
    return srcNode_var;
}

void SimpleMeshParentListMessage::setSrcNode(const TransportAddress& srcNode_var)
{
    this->srcNode_var = srcNode_var;
}

void SimpleMeshParentListMessage::setParentsArraySize(unsigned int size)
{
    ::TransportAddress *parents_var2 = (size==0) ? NULL : new ::TransportAddress[size];
    unsigned int sz = parents_arraysize < size ? parents_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        parents_var2[i] = this->parents_var[i];
    parents_arraysize = size;
    delete [] this->parents_var;
    this->parents_var = parents_var2;
}

unsigned int SimpleMeshParentListMessage::getParentsArraySize() const
{
    return parents_arraysize;
}

TransportAddress& SimpleMeshParentListMessage::getParents(unsigned int k)
{
    if (k>=parents_arraysize) throw cRuntimeError("Array of size %d indexed by %d", parents_arraysize, k);
    return parents_var[k];
}

void SimpleMeshParentListMessage::setParents(unsigned int k, const TransportAddress& parents_var)
{
    if (k>=parents_arraysize) throw cRuntimeError("Array of size %d indexed by %d", parents_arraysize, k);
    this->parents_var[k]=parents_var;
}

class SimpleMeshParentListMessageDescriptor : public cClassDescriptor
{
  public:
    SimpleMeshParentListMessageDescriptor();
    virtual ~SimpleMeshParentListMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(SimpleMeshParentListMessageDescriptor);

SimpleMeshParentListMessageDescriptor::SimpleMeshParentListMessageDescriptor() : cClassDescriptor("SimpleMeshParentListMessage", "BaseOverlayMessage")
{
}

SimpleMeshParentListMessageDescriptor::~SimpleMeshParentListMessageDescriptor()
{
}

bool SimpleMeshParentListMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<SimpleMeshParentListMessage *>(obj)!=NULL;
}

const char *SimpleMeshParentListMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int SimpleMeshParentListMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int SimpleMeshParentListMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISARRAY | FD_ISCOMPOUND,
    };
    return (field>=0 && field<3) ? fieldTypeFlags[field] : 0;
}

const char *SimpleMeshParentListMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "command",
        "srcNode",
        "parents",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int SimpleMeshParentListMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='c' && strcmp(fieldName, "command")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcNode")==0) return base+1;
    if (fieldName[0]=='p' && strcmp(fieldName, "parents")==0) return base+2;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *SimpleMeshParentListMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "TransportAddress",
        "TransportAddress",
    };
    return (field>=0 && field<3) ? fieldTypeStrings[field] : NULL;
}

const char *SimpleMeshParentListMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "SIMPLEMESHCommand";
            return NULL;
        default: return NULL;
    }
}

int SimpleMeshParentListMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshParentListMessage *pp = (SimpleMeshParentListMessage *)object; (void)pp;
    switch (field) {
        case 2: return pp->getParentsArraySize();
        default: return 0;
    }
}

std::string SimpleMeshParentListMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshParentListMessage *pp = (SimpleMeshParentListMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getCommand());
        case 1: {std::stringstream out; out << pp->getSrcNode(); return out.str();}
        case 2: {std::stringstream out; out << pp->getParents(i); return out.str();}
        default: return "";
    }
}

bool SimpleMeshParentListMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshParentListMessage *pp = (SimpleMeshParentListMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setCommand(string2long(value)); return true;
        default: return false;
    }
}

const char *SimpleMeshParentListMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        "TransportAddress",
        "TransportAddress",
    };
    return (field>=0 && field<3) ? fieldStructNames[field] : NULL;
}

void *SimpleMeshParentListMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshParentListMessage *pp = (SimpleMeshParentListMessage *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getSrcNode()); break;
        case 2: return (void *)(&pp->getParents(i)); break;
        default: return NULL;
    }
}

Register_Class(SimpleMeshReplaceMessage);

SimpleMeshReplaceMessage::SimpleMeshReplaceMessage(const char *name, int kind) : BaseOverlayMessage(name,kind)
{
    this->command_var = 0;
}

SimpleMeshReplaceMessage::SimpleMeshReplaceMessage(const SimpleMeshReplaceMessage& other) : BaseOverlayMessage()
{
    setName(other.getName());
    operator=(other);
}

SimpleMeshReplaceMessage::~SimpleMeshReplaceMessage()
{
}

SimpleMeshReplaceMessage& SimpleMeshReplaceMessage::operator=(const SimpleMeshReplaceMessage& other)
{
    if (this==&other) return *this;
    BaseOverlayMessage::operator=(other);
    this->command_var = other.command_var;
    this->srcNode_var = other.srcNode_var;
    this->nodeToBeReplaced_var = other.nodeToBeReplaced_var;
    return *this;
}

void SimpleMeshReplaceMessage::parsimPack(cCommBuffer *b)
{
    BaseOverlayMessage::parsimPack(b);
    doPacking(b,this->command_var);
    doPacking(b,this->srcNode_var);
    doPacking(b,this->nodeToBeReplaced_var);
}

void SimpleMeshReplaceMessage::parsimUnpack(cCommBuffer *b)
{
    BaseOverlayMessage::parsimUnpack(b);
    doUnpacking(b,this->command_var);
    doUnpacking(b,this->srcNode_var);
    doUnpacking(b,this->nodeToBeReplaced_var);
}

int SimpleMeshReplaceMessage::getCommand() const
{
    return command_var;
}

void SimpleMeshReplaceMessage::setCommand(int command_var)
{
    this->command_var = command_var;
}

TransportAddress& SimpleMeshReplaceMessage::getSrcNode()
{
    return srcNode_var;
}

void SimpleMeshReplaceMessage::setSrcNode(const TransportAddress& srcNode_var)
{
    this->srcNode_var = srcNode_var;
}

TransportAddress& SimpleMeshReplaceMessage::getNodeToBeReplaced()
{
    return nodeToBeReplaced_var;
}

void SimpleMeshReplaceMessage::setNodeToBeReplaced(const TransportAddress& nodeToBeReplaced_var)
{
    this->nodeToBeReplaced_var = nodeToBeReplaced_var;
}

class SimpleMeshReplaceMessageDescriptor : public cClassDescriptor
{
  public:
    SimpleMeshReplaceMessageDescriptor();
    virtual ~SimpleMeshReplaceMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(SimpleMeshReplaceMessageDescriptor);

SimpleMeshReplaceMessageDescriptor::SimpleMeshReplaceMessageDescriptor() : cClassDescriptor("SimpleMeshReplaceMessage", "BaseOverlayMessage")
{
}

SimpleMeshReplaceMessageDescriptor::~SimpleMeshReplaceMessageDescriptor()
{
}

bool SimpleMeshReplaceMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<SimpleMeshReplaceMessage *>(obj)!=NULL;
}

const char *SimpleMeshReplaceMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int SimpleMeshReplaceMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int SimpleMeshReplaceMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<3) ? fieldTypeFlags[field] : 0;
}

const char *SimpleMeshReplaceMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "command",
        "srcNode",
        "nodeToBeReplaced",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int SimpleMeshReplaceMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='c' && strcmp(fieldName, "command")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcNode")==0) return base+1;
    if (fieldName[0]=='n' && strcmp(fieldName, "nodeToBeReplaced")==0) return base+2;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *SimpleMeshReplaceMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "TransportAddress",
        "TransportAddress",
    };
    return (field>=0 && field<3) ? fieldTypeStrings[field] : NULL;
}

const char *SimpleMeshReplaceMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "SIMPLEMESHCommand";
            return NULL;
        default: return NULL;
    }
}

int SimpleMeshReplaceMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshReplaceMessage *pp = (SimpleMeshReplaceMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string SimpleMeshReplaceMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshReplaceMessage *pp = (SimpleMeshReplaceMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getCommand());
        case 1: {std::stringstream out; out << pp->getSrcNode(); return out.str();}
        case 2: {std::stringstream out; out << pp->getNodeToBeReplaced(); return out.str();}
        default: return "";
    }
}

bool SimpleMeshReplaceMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshReplaceMessage *pp = (SimpleMeshReplaceMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setCommand(string2long(value)); return true;
        default: return false;
    }
}

const char *SimpleMeshReplaceMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        "TransportAddress",
        "TransportAddress",
    };
    return (field>=0 && field<3) ? fieldStructNames[field] : NULL;
}

void *SimpleMeshReplaceMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    SimpleMeshReplaceMessage *pp = (SimpleMeshReplaceMessage *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getSrcNode()); break;
        case 2: return (void *)(&pp->getNodeToBeReplaced()); break;
        default: return NULL;
    }
}


