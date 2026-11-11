[vk::binding(0, 0)]
Texture2D textures2D[];

[vk::binding(0, 0)]
Texture2D<float> textures2DFloat[];

[vk::binding(0, 0)]
Texture2D<uint> textures2DUint[];

[vk::binding(0, 0)]
Texture2D<uint2> textures2DUint2[];

[vk::binding(0, 0)]
TextureCube texturesCube[];

[vk::binding(1, 0)]
RWTexture2D<uint2> storage2DUint2[];
