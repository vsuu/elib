
template<class corbaT, class localT>
struct corbaConvertHelper
{
    static void localToCorbaImpl(const localT &local, corbaT &corba)
    {
        corba = local;
    }
    static void corbaTolocalImpl(const corbaT& corba, localT &local)
    {
        local = corba;
    }

};


template<class corbaT, class localT>
inline void localToCorba(const localT & local, corbaT &corba)
{
    corbaConvertHelper<corbaT, localT>::localToCorbaImpl(local, corba);
}

template<class corbaT, class localT>
inline void corbaTolocal(const corbaT &corba, localT &local)
{
    corbaConvertHelper<corbaT, localT>::corbaTolocalImpl(corba, local);
}




#define CREATEITEMS1(corbaT,localT,corbaItem1,localItem1) \
static void localToCorbaImpl(const localT&local,corbaT &corba)\
{\
    localToCorba(local.localItem1,corba.corbaItem1);\
}\
static void corbaTolocalImpl(const corbaT &corba, localT&local)\
{\
    corbaTolocal(corba.corbaItem1,local.localItem1);\
}

#define CREATEITEMS2(corbaT,localT,corbaItem1,localItem1,corbaItem2,localItem2) \
static void localToCorbaImpl(const localT&local,corbaT &corba)\
{\
    localToCorba(local.localItem1,corba.corbaItem1);\
    localToCorba(local.localItem2,corba.corbaItem2);\
}\
static void corbaTolocalImpl(const corbaT &corba, localT&local)\
{\
    corbaTolocal(corba.corbaItem1,local.localItem1);\
    corbaTolocal(corba.corbaItem2,local.localItem2);\
}

#define CREATEITEMS3(corbaT,localT,corbaItem1,localItem1,corbaItem2,localItem2,corbaItem3,localItem3) \
static void localToCorbaImpl(const localT&local,corbaT &corba)\
{\
    localToCorba(local.localItem1,corba.corbaItem1);\
    localToCorba(local.localItem2,corba.corbaItem2);\
    localToCorba(local.localItem3,corba.corbaItem3);\
}\
static void corbaTolocalImpl(const corbaT &corba, localT&local)\
{\
    corbaTolocal(corba.corbaItem1,local.localItem1);\
    corbaTolocal(corba.corbaItem2,local.localItem2);\
    corbaTolocal(corba.corbaItem3,local.localItem3);\
}

#define CREATEITEMS4(corbaT,localT,corbaItem1,localItem1,corbaItem2,localItem2,corbaItem3,localItem3,corbaItem4,localItem4) \
static void localToCorbaImpl(const localT&local,corbaT &corba)\
{\
    localToCorba(local.localItem1,corba.corbaItem1);\
    localToCorba(local.localItem2,corba.corbaItem2);\
    localToCorba(local.localItem3,corba.corbaItem3);\
    localToCorba(local.localItem4,corba.corbaItem4);\
}\
static void corbaTolocalImpl(const corbaT &corba, localT&local)\
{\
    corbaTolocal(corba.corbaItem1,local.localItem1);\
    corbaTolocal(corba.corbaItem2,local.localItem2);\
    corbaTolocal(corba.corbaItem3,local.localItem3);\
    corbaTolocal(corba.corbaItem4,local.localItem4);\
}

#define CREATEITEMS5(corbaT,localT,corbaItem1,localItem1,corbaItem2,localItem2,corbaItem3,localItem3,corbaItem4,localItem4,corbaItem5,localItem5) \
static void localToCorbaImpl(const localT & local,corbaT &corba)\
{\
    localToCorba(local.localItem1,corba.corbaItem1);\
    localToCorba(local.localItem2,corba.corbaItem2);\
    localToCorba(local.localItem3,corba.corbaItem3);\
    localToCorba(local.localItem4,corba.corbaItem4);\
    localToCorba(local.localItem5,corba.corbaItem5);\
}\
static void corbaTolocalImpl(const corbaT &corba, localT & local)\
{\
    corbaTolocal(corba.corbaItem1,local.localItem1);\
    corbaTolocal(corba.corbaItem2,local.localItem2);\
    corbaTolocal(corba.corbaItem3,local.localItem3);\
    corbaTolocal(corba.corbaItem4,local.localItem4);\
    corbaTolocal(corba.corbaItem5,local.localItem5);\
}






#define CORBACONVERT(corbaT,localT,itemNum,...) template<> \
struct corbaConvertHelper<corbaT,localT> {\
    CREATEITEMS##itemNum(corbaT,localT,__VA_ARGS__) \
};

struct corbademo
{
    int x;
    int y;
    int z;
};

struct localdemo
{
    int a;
    int b;
    int c;
};


CORBACONVERT(corbademo, localdemo, 3, x, a, y, b, z, c)
