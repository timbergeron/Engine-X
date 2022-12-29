// Baker: We do not actually use this file.  Because I want gamehacks highly localized and not globally available.
//        Still I tend to put the crap here so I know of them.

modhint_t GameHacks_IsSpecialQuakeAliasModel (const char *model_name);
qbool GameHacks_IsNotQuakeBoxModel (const char *bspname);
qbool GameHacks_Is_EXMY_Map (const char *mapname);
qbool Alias_SetSpecialLighting(const modhint_t my_modhint, qbool *set_full_light, qbool *set_noshadow, float *set_shadelight, float *set_ambientlight);
