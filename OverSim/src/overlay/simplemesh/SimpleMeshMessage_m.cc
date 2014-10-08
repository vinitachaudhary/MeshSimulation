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
    e->insert(POWER_REQUEST, "POWER_REQUEST");
    e->insert(POWER_RESPONSE, "POWER_RESPONSE");
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


