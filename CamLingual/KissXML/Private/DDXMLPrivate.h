#import "DDXML.h"


// We can't rely solely on NSAssert, because many developers disable them for release builds.
// Our API contract requires us to keep these assertions intact.
#define DDXMLAssert(condition, desc, ...)                                                                 \
  do{                                                                                                     \
    if(!(condition))                                                                                      \
    {                                                                                                     \
      [[NSAssertionHandler currentHandler] handleFailureInMethod:_cmd                                     \
                                                          object:self                                     \
                                                            file:[NSString stringWithUTF8String:__FILE__] \
                                                      lineNumber:__LINE__                                 \
                                                     description:(desc), ##__VA_ARGS__];                  \
    }                                                                                                     \
  }while(NO)

#define DDLastErrorKey @"DDXML:LastError"

/**
 * DDXMLNode can represent several underlying types, such as xmlNodePtr, xmlDocPtr, xmlAttrPtr, xmlNsPtr, etc.
 * All of these are pointers to structures, and all of those structures start with a pointer, and a type.
 * The xmlKind struct is used as a generic structure, and a stepping stone.
 * We use it to check the type of a structure, and then perform the appropriate cast.
 * 
 * For example:
 * if(genericPtr->type == XML_ATTRIBUTE_NODE)
 * {
 *     xmlAttrPtr attr = (xmlAttrPtr)genericPtr;
 *     // Do something with attr
 * }
**/
struct _xmlKind {
	void * ignore;
	xmlElementType type;
};
typedef struct _xmlKind *xmlKindPtr;

/**
 * Most xml types all start with this standard structure. In fact, all do except the xmlNsPtr.
 * We will occasionally take advantage of this to simplify code when the code wouldn't vary from type to type.
 * Obviously, you cannnot cast a xmlNsPtr to a xmlStdPtr.
**/
struct _xmlStd {
	void * _private;
	xmlElementType type;
	const xmlChar *name;
	struct _xmlNode *children;
	struct _xmlNode *last;
	struct _xmlNode *parent;
	struct _xmlStd *next;
	struct _xmlStd *prev;
	struct _xmlDoc *doc;
};
typedef struct _xmlStd *xmlStdPtr;


NS_INLINE BOOL IsXmlAttrPtr(void *kindPtr)
{
	return ((xmlKindPtr)kindPtr)->type == XML_ATTRIBUTE_NODE;
}

NS_INLINE BOOL IsXmlNodePtr(void *kindPtr)
{
	switch (((xmlKindPtr)kindPtr)->type)
	{
		case XML_ELEMENT_NODE       :
		case XML_PI_NODE            : 
		case XML_COMMENT_NODE       : 
		case XML_TEXT_NODE          : 
		case XML_CDATA_SECTION_NODE : return YES;
		default                     : return NO;
	}
}

NS_INLINE BOOL IsXmlDocPtr(void *kindPtr)
{
    return ((xmlKindPtr)kindPtr)->type == XML_DOCUMENT_NODE
    || ((xmlKindPtr)kindPtr)->type == XML_HTML_DOCUMENT_NODE;
    
//	return ((xmlKindPtr)kindPtr)->type == XML_DOCUMENT_NODE;
}

NS_INLINE BOOL IsXmlDtdPtr(void *kindPtr)
{
	return ((xmlKindPtr)kindPtr)->type == XML_DTD_NODE;
}

NS_INLINE BOOL IsXmlNsPtr(void *kindPtr)
{
	return ((xmlKindPtr)kindPtr)->type == XML_NAMESPACE_DECL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface DDXMLNamespaceNode : DDXMLNode
{
	// The xmlNsPtr type doesn't store a reference to it's parent.
	// This is here to fix the problem, and make this class more compatible with the NSXML classes.
	xmlNodePtr nsParentPtr;
}

+ (id)nodeWithNsPrimitive:(xmlNsPtr)ns nsParent:(xmlNodePtr)parent freeOnDealloc:(BOOL)flag;
- (id)initWithNsPrimitive:(xmlNsPtr)ns nsParent:(xmlNodePtr)parent freeOnDealloc:(BOOL)flag;

- (xmlNodePtr)nsParentPtr;
- (void)setNsParentPtr:(xmlNodePtr)parentPtr;

// Overrides several methods in DDXMLNode

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface DDXMLAttributeNode : DDXMLNode

+ (id)nodeWithAttrPrimitive:(xmlAttrPtr)attr freeOnDealloc:(BOOL)flag;
- (id)initWithAttrPrimitive:(xmlAttrPtr)attr freeOnDealloc:(BOOL)flag;

// Overrides several methods in DDXMLNode

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface DDXMLNode (PrivateAPI)

+ (id)nodeWithUnknownPrimitive:(xmlKindPtr)kindPtr freeOnDealloc:(BOOL)flag;

+ (id)nodeWithPrimitive:(xmlKindPtr)kindPtr freeOnDealloc:(BOOL)flag;
- (id)initWithPrimitive:(xmlKindPtr)kindPtr freeOnDealloc:(BOOL)flag;

- (BOOL)hasParent;

+ (void)recursiveStripDocPointersFromNode:(xmlNodePtr)node;

+ (void)detachAttribute:(xmlAttrPtr)attr fromNode:(xmlNodePtr)node;
+ (void)removeAttribute:(xmlAttrPtr)attr fromNode:(xmlNodePtr)node;
+ (void)removeAllAttributesFromNode:(xmlNodePtr)node;

+ (void)detachNamespace:(xmlNsPtr)ns fromNode:(xmlNodePtr)node;
+ (void)removeNamespace:(xmlNsPtr)ns fromNode:(xmlNodePtr)node;
+ (void)removeAllNamespacesFromNode:(xmlNodePtr)node;

+ (void)detachChild:(xmlNodePtr)child fromNode:(xmlNodePtr)node;
+ (void)removeChild:(xmlNodePtr)child fromNode:(xmlNodePtr)node;
+ (void)removeAllChildrenFromNode:(xmlNodePtr)node;

- (void)nodeFree;

+ (NSError *)lastError;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface DDXMLElement (PrivateAPI)

+ (id)nodeWithElementPrimitive:(xmlNodePtr)node freeOnDealloc:(BOOL)flag;
- (id)initWithElementPrimitive:(xmlNodePtr)node freeOnDealloc:(BOOL)flag;

- (NSArray *)elementsForName:(NSString *)name uri:(NSString *)URI;

+ (DDXMLNode *)resolveNamespaceForPrefix:(NSString *)prefix atNode:(xmlNodePtr)nodePtr;
+ (NSString *)resolvePrefixForURI:(NSString *)uri atNode:(xmlNodePtr)nodePtr;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface DDXMLDocument (PrivateAPI)

+ (id)nodeWithDocPrimitive:(xmlDocPtr)doc freeOnDealloc:(BOOL)flag;
- (id)initWithDocPrimitive:(xmlDocPtr)doc freeOnDealloc:(BOOL)flag;

@end
