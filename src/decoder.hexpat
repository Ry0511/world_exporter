using Level;
using UObject;

struct FName {
  s32 index;
  s32 number;
};

struct TArray<T> {
  T* data : u32;
  s32 len;
  s32 capacity;
};

struct Ptr<T> { T* ptr : s32; };
struct TArrayWithOwner<T> : TArray<T> { UObject* owner : s32; };

struct TArrayVoid : TArray<u32> {};
struct FString : TArray<char16> {};

struct FRenderResource {
    u32* vftable : u32;
    u32 _1[3];
    u32 bIsInitialised;
};

struct FURL {
  FString protocol;
  FString host;
  s32 port;
  FString map;
  TArrayVoid op;
  FString portal;
  s32 valid;
};

struct UObject {
  u32*  vftable : s32;
  u32*  HashNext : s32;
  u64   ObjectFlags;
  u32*  HashOuterNext : s32;
  u32*  StateFrame : s32;
  u64*  _Linker : s32;
  u32*  _LinkerIndex : s32;
  s32   InternalIndex;
  s32   NetIndex;
  u64*  Outer : s32;
  FName Name;
  u64*  Class : s32;
  u64*  ObjectArchetype : s32;
};

struct Model {
  UObject _This_UObject;
  s32* polys : s32;
  TArrayWithOwner<u32> nodes;
  TArrayWithOwner<u32> verts;
  TArrayWithOwner<u32> vectors;
  TArrayWithOwner<u32> points;
  TArrayWithOwner<u32> surfs;
  TArray<s32> leaf_hulls;
  TArray<s32> leaves;
  TArray<s32> portal_nodes;
};

struct Level {
  UObject _This_UObject;
  TArrayWithOwner<Ptr<UObject>> actors;
  FURL url;
  Model* model : s32;
};

struct UWorld {
  UObject _This_UObject;
  s32 FNetworkNotifyVTable;
  s32 Scene;
  TArray<Ptr<Level>> Levels;
  Level* PersistentLevel : s32;
};

struct TLinkedList<T> {
  T element;
  u32* NextLink : u32;
  u32* PrevLink : u32;
};

struct FVertexBuffer {
    u32 vftable;
    TLinkedList<u32> ResourceLink; // FRenderResouce* linked list
    u32 bInitialised;
    u32 VertexBufferRHI;
};

struct FVertex {
    float X;
    float Y;
    float Z;
};

struct FStaticMeshVertexBuffer : FVertexBuffer {
    u8* VertexData : u32;
    u32 NumTexCoords;
    u8* Data : u32;
    u32 Stride;
    u32 NumVertices;
    u32 bUseFullPrecisionUVs;
};

struct FPositionVertexBuffer : FVertexBuffer {
    u32* VertexData : u32;
    u8* Data : u32;
    u32 Stride;
    u32 NumVertices;
};

struct FColourVertexBuffer : FVertexBuffer {
    u32* VertexData : u32;
    u8* Data : u32;
    u32 Stride;
    u32 NumVertices;
};

struct FFragmentRange {
  u32 BaseIndex;
  u32 NumPrimitives;
};

struct FStaticMeshElement {
    u32* Material : u32;
    FString Name;
    u32 bEnableCollision;
    u32 bOldEnableCollision;
    u32 bEnableShadowCasting;
    u32 FirstIndex;
    u32 NumTriangles;
    u32 MinVertexIndex;
    u32 MaxVertexIndex;
    u32 MaterialIndex;
    TArray<FFragmentRange> Fragments;
    u32* PlatformData : u32;
};

struct FStaticMeshRenderData {
    FStaticMeshVertexBuffer VertexBuffer;
    FPositionVertexBuffer PositionVertexBuffer;
    FColourVertexBuffer ColourVertexBuffer;
    u32 NumVertices;
    u32 _4[22];
    TArray<FStaticMeshElement> SubMeshes;
};

struct UStaticMesh {
    UObject _THIS_UObject;
    TArray<Ptr<FStaticMeshRenderData>> LodModels;
    TArray<u32> LodInfo;
    float LodDistanceRatio;
    float LodMaxRange;
};

struct FModelVertexBuffer {
  FVertexBuffer _VertexBuffer;
  TArray<FModelVertex> Vertices;
};

struct UModel {
    UObject _THIS_UObject;
    TArrayWithOwner<u32> _1[5];
    TArray<u32> _2[4];
    u8 _3[20];
    FModelVertexBuffer VertexBuffer;
};

UModel _1 @ 0x1ce26c18;