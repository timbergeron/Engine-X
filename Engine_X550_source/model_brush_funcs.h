#ifndef __MODEL_BRUSH_FUNCS_H__
#define __MODEL_BRUSH_FUNCS_H__


// Brush model stuff

void Mod_BrushModel_Clear_Vis (void);

mleaf_t *Mod_PointInLeaf (const float *p, model_t *model);
byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model);
texture_t *R_TextureAnimation (texture_t *base, int framer);

#endif // __MODEL_BRUSH_FUNCS_H__