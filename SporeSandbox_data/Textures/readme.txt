In order to save memory and texture slots, we merge some of the PBR maps.
The current shader uses 4 textures:
 1. albedo + roughness: albedo RGB, roughness A, save it as DXT5 (BC3)
 2. normal map: RBG, save it as DXT1 (BC1)
 3. metallic: in alpha channel, save as A8
 4. ambient occlusion: in alpha channel, save as A8

To save things as A8, create empty image, add channel mask, 
and in the channels paste the image there.

0 is not metallic, 1 is metallic