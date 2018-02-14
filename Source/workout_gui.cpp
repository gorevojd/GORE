#include "workout_gui.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"


inline v4 GUIColor255(int R, int G, int B) {
	float OneOver255 = 1.0f / 255.0f;
	v4 Result = V4(R, G, B, 1.0f);
	Result.r *= OneOver255;
	Result.g *= OneOver255;
	Result.b *= OneOver255;

	return(Result);
}

inline int GUIIntFromHexChar(char C) {
	int Result = 0;

	if (C >= 'a' && C <= 'f') {
		C += 'A' - 'a';
	}

	if (C >= '0' && C <= '9') {
		Result = C - '0';
	}

	if (C >= 'A' && C <= 'F') {
		Result = C + 10 - 'A';
	}

	return(Result);
}

inline v4 GUIColorHex(char* Str) {
	float OneOver255 = 1.0f / 255.0f;

	v4 Result;

	Assert(Str[0] == '#');

	int R, G, B;
	R = GUIIntFromHexChar(Str[1]) * 16 + GUIIntFromHexChar(Str[2]);
	G = GUIIntFromHexChar(Str[3]) * 16 + GUIIntFromHexChar(Str[4]);
	B = GUIIntFromHexChar(Str[5]) * 16 + GUIIntFromHexChar(Str[6]);

	Result = V4(R, G, B, 1.0f);

	Result.r *= OneOver255;
	Result.g *= OneOver255;
	Result.b *= OneOver255;

	return(Result);
}

inline gui_color_slot GUICreateColorSlot(gui_state* GUIState, v4 Color, char* Name) {
	gui_color_slot Res = {};

	Res.Color = Color;
	Res.Name = PushString(&GUIState->GUIMem, Name);
	CopyStrings(Res.Name, Name);

	char* At = Res.Name;
	while (*At) {

		if (*At >= 'a' && *At <= 'z') {
			*At = *At - 'a' + 'A';
		}

		At++;
	}

	return(Res);
}

inline v4 GUIGetColor(gui_state* GUIState, u32 ColorIndex) {
	v4 Result = GUIState->ColorTable[ColorIndex].Color;

	return(Result);
}


#if GUI_EXT_COLORS_ENABLED
static void GUIExtInitColors(gui_state* GUIState) {
	GUIState->ColorTable[GUIColorExt_AliceBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#f0f8ff"), "AliceBlue");
	GUIState->ColorTable[GUIColorExt_AntiqueWhite] = GUICreateColorSlot(GUIState, GUIColorHex("#faebd7"), "AntiqueWhite");
	GUIState->ColorTable[GUIColorExt_AntiqueWhite1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffefdb"), "AntiqueWhite1");
	GUIState->ColorTable[GUIColorExt_AntiqueWhite2] = GUICreateColorSlot(GUIState, GUIColorHex("#eedfcc"), "AntiqueWhite2");
	GUIState->ColorTable[GUIColorExt_AntiqueWhite3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdc0b0"), "AntiqueWhite3");
	GUIState->ColorTable[GUIColorExt_AntiqueWhite4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8378"), "AntiqueWhite4");
	GUIState->ColorTable[GUIColorExt_aquamarine1] = GUICreateColorSlot(GUIState, GUIColorHex("#7fffd4"), "aquamarine1");
	GUIState->ColorTable[GUIColorExt_aquamarine2] = GUICreateColorSlot(GUIState, GUIColorHex("#76eec6"), "aquamarine2");
	GUIState->ColorTable[GUIColorExt_aquamarine4] = GUICreateColorSlot(GUIState, GUIColorHex("#458b74"), "aquamarine4");
	GUIState->ColorTable[GUIColorExt_azure1] = GUICreateColorSlot(GUIState, GUIColorHex("#f0ffff"), "azure1");
	GUIState->ColorTable[GUIColorExt_azure2] = GUICreateColorSlot(GUIState, GUIColorHex("#e0eeee"), "azure2");
	GUIState->ColorTable[GUIColorExt_azure3] = GUICreateColorSlot(GUIState, GUIColorHex("#c1cdcd"), "azure3");
	GUIState->ColorTable[GUIColorExt_azure4] = GUICreateColorSlot(GUIState, GUIColorHex("#838b8b"), "azure4");
	GUIState->ColorTable[GUIColorExt_beige] = GUICreateColorSlot(GUIState, GUIColorHex("#f5f5dc"), "beige");
	GUIState->ColorTable[GUIColorExt_bisque1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffe4c4"), "bisque1");
	GUIState->ColorTable[GUIColorExt_bisque2] = GUICreateColorSlot(GUIState, GUIColorHex("#eed5b7"), "bisque2");
	GUIState->ColorTable[GUIColorExt_bisque3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdb79e"), "bisque3");
	GUIState->ColorTable[GUIColorExt_bisque4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b7d6b"), "bisque4");
	GUIState->ColorTable[GUIColorExt_black] = GUICreateColorSlot(GUIState, GUIColorHex("#000000"), "black");
	GUIState->ColorTable[GUIColorExt_BlanchedAlmond] = GUICreateColorSlot(GUIState, GUIColorHex("#ffebcd"), "BlanchedAlmond");
	GUIState->ColorTable[GUIColorExt_blue1] = GUICreateColorSlot(GUIState, GUIColorHex("#0000ff"), "blue1");
	GUIState->ColorTable[GUIColorExt_blue2] = GUICreateColorSlot(GUIState, GUIColorHex("#0000ee"), "blue2");
	GUIState->ColorTable[GUIColorExt_blue4] = GUICreateColorSlot(GUIState, GUIColorHex("#00008b"), "blue4");
	GUIState->ColorTable[GUIColorExt_BlueViolet] = GUICreateColorSlot(GUIState, GUIColorHex("#8a2be2"), "BlueViolet");
	GUIState->ColorTable[GUIColorExt_brown] = GUICreateColorSlot(GUIState, GUIColorHex("#a52a2a"), "brown");
	GUIState->ColorTable[GUIColorExt_brown1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff4040"), "brown1");
	GUIState->ColorTable[GUIColorExt_brown2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee3b3b"), "brown2");
	GUIState->ColorTable[GUIColorExt_brown3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd3333"), "brown3");
	GUIState->ColorTable[GUIColorExt_brown4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b2323"), "brown4");
	GUIState->ColorTable[GUIColorExt_burlywood] = GUICreateColorSlot(GUIState, GUIColorHex("#deb887"), "burlywood");
	GUIState->ColorTable[GUIColorExt_burlywood1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffd39b"), "burlywood1");
	GUIState->ColorTable[GUIColorExt_burlywood2] = GUICreateColorSlot(GUIState, GUIColorHex("#eec591"), "burlywood2");
	GUIState->ColorTable[GUIColorExt_burlywood3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdaa7d"), "burlywood3");
	GUIState->ColorTable[GUIColorExt_burlywood4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b7355"), "burlywood4");
	GUIState->ColorTable[GUIColorExt_CadetBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#5f9ea0"), "CadetBlue");
	GUIState->ColorTable[GUIColorExt_CadetBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#98f5ff"), "CadetBlue1");
	GUIState->ColorTable[GUIColorExt_CadetBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#8ee5ee"), "CadetBlue2");
	GUIState->ColorTable[GUIColorExt_CadetBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#7ac5cd"), "CadetBlue3");
	GUIState->ColorTable[GUIColorExt_CadetBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#53868b"), "CadetBlue4");
	GUIState->ColorTable[GUIColorExt_chartreuse1] = GUICreateColorSlot(GUIState, GUIColorHex("#7fff00"), "chartreuse1");
	GUIState->ColorTable[GUIColorExt_chartreuse2] = GUICreateColorSlot(GUIState, GUIColorHex("#76ee00"), "chartreuse2");
	GUIState->ColorTable[GUIColorExt_chartreuse3] = GUICreateColorSlot(GUIState, GUIColorHex("#66cd00"), "chartreuse3");
	GUIState->ColorTable[GUIColorExt_chartreuse4] = GUICreateColorSlot(GUIState, GUIColorHex("#458b00"), "chartreuse4");
	GUIState->ColorTable[GUIColorExt_chocolate] = GUICreateColorSlot(GUIState, GUIColorHex("#d2691e"), "chocolate");
	GUIState->ColorTable[GUIColorExt_chocolate1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff7f24"), "chocolate1");
	GUIState->ColorTable[GUIColorExt_chocolate2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee7621"), "chocolate2");
	GUIState->ColorTable[GUIColorExt_chocolate3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd661d"), "chocolate3");
	GUIState->ColorTable[GUIColorExt_coral] = GUICreateColorSlot(GUIState, GUIColorHex("#ff7f50"), "coral");
	GUIState->ColorTable[GUIColorExt_coral1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff7256"), "coral1");
	GUIState->ColorTable[GUIColorExt_coral2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee6a50"), "coral2");
	GUIState->ColorTable[GUIColorExt_coral3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd5b45"), "coral3");
	GUIState->ColorTable[GUIColorExt_coral4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b3e2f"), "coral4");
	GUIState->ColorTable[GUIColorExt_CornflowerBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#6495ed"), "CornflowerBlue");
	GUIState->ColorTable[GUIColorExt_cornsilk1] = GUICreateColorSlot(GUIState, GUIColorHex("#fff8dc"), "cornsilk1");
	GUIState->ColorTable[GUIColorExt_cornsilk2] = GUICreateColorSlot(GUIState, GUIColorHex("#eee8cd"), "cornsilk2");
	GUIState->ColorTable[GUIColorExt_cornsilk3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdc8b1"), "cornsilk3");
	GUIState->ColorTable[GUIColorExt_cornsilk4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8878"), "cornsilk4");
	GUIState->ColorTable[GUIColorExt_cyan1] = GUICreateColorSlot(GUIState, GUIColorHex("#00ffff"), "cyan1");
	GUIState->ColorTable[GUIColorExt_cyan2] = GUICreateColorSlot(GUIState, GUIColorHex("#00eeee"), "cyan2");
	GUIState->ColorTable[GUIColorExt_cyan3] = GUICreateColorSlot(GUIState, GUIColorHex("#00cdcd"), "cyan3");
	GUIState->ColorTable[GUIColorExt_cyan4] = GUICreateColorSlot(GUIState, GUIColorHex("#008b8b"), "cyan4");
	GUIState->ColorTable[GUIColorExt_DarkGoldenrod] = GUICreateColorSlot(GUIState, GUIColorHex("#b8860b"), "DarkGoldenrod");
	GUIState->ColorTable[GUIColorExt_DarkGoldenrod1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffb90f"), "DarkGoldenrod1");
	GUIState->ColorTable[GUIColorExt_DarkGoldenrod2] = GUICreateColorSlot(GUIState, GUIColorHex("#eead0e"), "DarkGoldenrod2");
	GUIState->ColorTable[GUIColorExt_DarkGoldenrod3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd950c"), "DarkGoldenrod3");
	GUIState->ColorTable[GUIColorExt_DarkGoldenrod4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b6508"), "DarkGoldenrod4");
	GUIState->ColorTable[GUIColorExt_DarkGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#006400"), "DarkGreen");
	GUIState->ColorTable[GUIColorExt_DarkKhaki] = GUICreateColorSlot(GUIState, GUIColorHex("#bdb76b"), "DarkKhaki");
	GUIState->ColorTable[GUIColorExt_DarkOliveGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#556b2f"), "DarkOliveGreen");
	GUIState->ColorTable[GUIColorExt_DarkOliveGreen1] = GUICreateColorSlot(GUIState, GUIColorHex("#caff70"), "DarkOliveGreen1");
	GUIState->ColorTable[GUIColorExt_DarkOliveGreen2] = GUICreateColorSlot(GUIState, GUIColorHex("#bcee68"), "DarkOliveGreen2");
	GUIState->ColorTable[GUIColorExt_DarkOliveGreen3] = GUICreateColorSlot(GUIState, GUIColorHex("#a2cd5a"), "DarkOliveGreen3");
	GUIState->ColorTable[GUIColorExt_DarkOliveGreen4] = GUICreateColorSlot(GUIState, GUIColorHex("#6e8b3d"), "DarkOliveGreen4");
	GUIState->ColorTable[GUIColorExt_DarkOrange] = GUICreateColorSlot(GUIState, GUIColorHex("#ff8c00"), "DarkOrange");
	GUIState->ColorTable[GUIColorExt_DarkOrange1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff7f00"), "DarkOrange1");
	GUIState->ColorTable[GUIColorExt_DarkOrange2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee7600"), "DarkOrange2");
	GUIState->ColorTable[GUIColorExt_DarkOrange3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd6600"), "DarkOrange3");
	GUIState->ColorTable[GUIColorExt_DarkOrange4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b4500"), "DarkOrange4");
	GUIState->ColorTable[GUIColorExt_DarkOrchid] = GUICreateColorSlot(GUIState, GUIColorHex("#9932cc"), "DarkOrchid");
	GUIState->ColorTable[GUIColorExt_DarkOrchid1] = GUICreateColorSlot(GUIState, GUIColorHex("#bf3eff"), "DarkOrchid1");
	GUIState->ColorTable[GUIColorExt_DarkOrchid2] = GUICreateColorSlot(GUIState, GUIColorHex("#b23aee"), "DarkOrchid2");
	GUIState->ColorTable[GUIColorExt_DarkOrchid3] = GUICreateColorSlot(GUIState, GUIColorHex("#9a32cd"), "DarkOrchid3");
	GUIState->ColorTable[GUIColorExt_DarkOrchid4] = GUICreateColorSlot(GUIState, GUIColorHex("#68228b"), "DarkOrchid4");
	GUIState->ColorTable[GUIColorExt_DarkSalmon] = GUICreateColorSlot(GUIState, GUIColorHex("#e9967a"), "DarkSalmon");
	GUIState->ColorTable[GUIColorExt_DarkSeaGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#8fbc8f"), "DarkSeaGreen");
	GUIState->ColorTable[GUIColorExt_DarkSeaGreen1] = GUICreateColorSlot(GUIState, GUIColorHex("#c1ffc1"), "DarkSeaGreen1");
	GUIState->ColorTable[GUIColorExt_DarkSeaGreen2] = GUICreateColorSlot(GUIState, GUIColorHex("#b4eeb4"), "DarkSeaGreen2");
	GUIState->ColorTable[GUIColorExt_DarkSeaGreen3] = GUICreateColorSlot(GUIState, GUIColorHex("#9bcd9b"), "DarkSeaGreen3");
	GUIState->ColorTable[GUIColorExt_DarkSeaGreen4] = GUICreateColorSlot(GUIState, GUIColorHex("#698b69"), "DarkSeaGreen4");
	GUIState->ColorTable[GUIColorExt_DarkSlateBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#483d8b"), "DarkSlateBlue");
	GUIState->ColorTable[GUIColorExt_DarkSlateGray] = GUICreateColorSlot(GUIState, GUIColorHex("#2f4f4f"), "DarkSlateGray");
	GUIState->ColorTable[GUIColorExt_DarkSlateGray1] = GUICreateColorSlot(GUIState, GUIColorHex("#97ffff"), "DarkSlateGray1");
	GUIState->ColorTable[GUIColorExt_DarkSlateGray2] = GUICreateColorSlot(GUIState, GUIColorHex("#8deeee"), "DarkSlateGray2");
	GUIState->ColorTable[GUIColorExt_DarkSlateGray3] = GUICreateColorSlot(GUIState, GUIColorHex("#79cdcd"), "DarkSlateGray3");
	GUIState->ColorTable[GUIColorExt_DarkSlateGray4] = GUICreateColorSlot(GUIState, GUIColorHex("#528b8b"), "DarkSlateGray4");
	GUIState->ColorTable[GUIColorExt_DarkTurquoise] = GUICreateColorSlot(GUIState, GUIColorHex("#00ced1"), "DarkTurquoise");
	GUIState->ColorTable[GUIColorExt_DarkViolet] = GUICreateColorSlot(GUIState, GUIColorHex("#9400d3"), "DarkViolet");
	GUIState->ColorTable[GUIColorExt_DeepPink1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff1493"), "DeepPink1");
	GUIState->ColorTable[GUIColorExt_DeepPink2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee1289"), "DeepPink2");
	GUIState->ColorTable[GUIColorExt_DeepPink3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd1076"), "DeepPink3");
	GUIState->ColorTable[GUIColorExt_DeepPink4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b0a50"), "DeepPink4");
	GUIState->ColorTable[GUIColorExt_DeepSkyBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#00bfff"), "DeepSkyBlue1");
	GUIState->ColorTable[GUIColorExt_DeepSkyBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#00b2ee"), "DeepSkyBlue2");
	GUIState->ColorTable[GUIColorExt_DeepSkyBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#009acd"), "DeepSkyBlue3");
	GUIState->ColorTable[GUIColorExt_DeepSkyBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#00688b"), "DeepSkyBlue4");
	GUIState->ColorTable[GUIColorExt_DimGray] = GUICreateColorSlot(GUIState, GUIColorHex("#696969"), "DimGray");
	GUIState->ColorTable[GUIColorExt_DodgerBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#1e90ff"), "DodgerBlue1");
	GUIState->ColorTable[GUIColorExt_DodgerBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#1c86ee"), "DodgerBlue2");
	GUIState->ColorTable[GUIColorExt_DodgerBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#1874cd"), "DodgerBlue3");
	GUIState->ColorTable[GUIColorExt_DodgerBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#104e8b"), "DodgerBlue4");
	GUIState->ColorTable[GUIColorExt_firebrick] = GUICreateColorSlot(GUIState, GUIColorHex("#b22222"), "firebrick");
	GUIState->ColorTable[GUIColorExt_firebrick1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff3030"), "firebrick1");
	GUIState->ColorTable[GUIColorExt_firebrick2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee2c2c"), "firebrick2");
	GUIState->ColorTable[GUIColorExt_firebrick3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd2626"), "firebrick3");
	GUIState->ColorTable[GUIColorExt_firebrick4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b1a1a"), "firebrick4");
	GUIState->ColorTable[GUIColorExt_FloralWhite] = GUICreateColorSlot(GUIState, GUIColorHex("#fffaf0"), "FloralWhite");
	GUIState->ColorTable[GUIColorExt_ForestGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#228b22"), "ForestGreen");
	GUIState->ColorTable[GUIColorExt_gainsboro] = GUICreateColorSlot(GUIState, GUIColorHex("#dcdcdc"), "gainsboro");
	GUIState->ColorTable[GUIColorExt_GhostWhite] = GUICreateColorSlot(GUIState, GUIColorHex("#f8f8ff"), "GhostWhite");
	GUIState->ColorTable[GUIColorExt_gold1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffd700"), "gold1");
	GUIState->ColorTable[GUIColorExt_gold2] = GUICreateColorSlot(GUIState, GUIColorHex("#eec900"), "gold2");
	GUIState->ColorTable[GUIColorExt_gold3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdad00"), "gold3");
	GUIState->ColorTable[GUIColorExt_gold4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b7500"), "gold4");
	GUIState->ColorTable[GUIColorExt_goldenrod] = GUICreateColorSlot(GUIState, GUIColorHex("#daa520"), "goldenrod");
	GUIState->ColorTable[GUIColorExt_goldenrod1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffc125"), "goldenrod1");
	GUIState->ColorTable[GUIColorExt_goldenrod2] = GUICreateColorSlot(GUIState, GUIColorHex("#eeb422"), "goldenrod2");
	GUIState->ColorTable[GUIColorExt_goldenrod3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd9b1d"), "goldenrod3");
	GUIState->ColorTable[GUIColorExt_goldenrod4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b6914"), "goldenrod4");
	GUIState->ColorTable[GUIColorExt_gray] = GUICreateColorSlot(GUIState, GUIColorHex("#bebebe"), "gray");
	GUIState->ColorTable[GUIColorExt_gray1] = GUICreateColorSlot(GUIState, GUIColorHex("#030303"), "gray1");
	GUIState->ColorTable[GUIColorExt_gray10] = GUICreateColorSlot(GUIState, GUIColorHex("#1a1a1a"), "gray10");
	GUIState->ColorTable[GUIColorExt_gray11] = GUICreateColorSlot(GUIState, GUIColorHex("#1c1c1c"), "gray11");
	GUIState->ColorTable[GUIColorExt_gray12] = GUICreateColorSlot(GUIState, GUIColorHex("#1f1f1f"), "gray12");
	GUIState->ColorTable[GUIColorExt_gray13] = GUICreateColorSlot(GUIState, GUIColorHex("#212121"), "gray13");
	GUIState->ColorTable[GUIColorExt_gray14] = GUICreateColorSlot(GUIState, GUIColorHex("#242424"), "gray14");
	GUIState->ColorTable[GUIColorExt_gray15] = GUICreateColorSlot(GUIState, GUIColorHex("#262626"), "gray15");
	GUIState->ColorTable[GUIColorExt_gray16] = GUICreateColorSlot(GUIState, GUIColorHex("#292929"), "gray16");
	GUIState->ColorTable[GUIColorExt_gray17] = GUICreateColorSlot(GUIState, GUIColorHex("#2b2b2b"), "gray17");
	GUIState->ColorTable[GUIColorExt_gray18] = GUICreateColorSlot(GUIState, GUIColorHex("#2e2e2e"), "gray18");
	GUIState->ColorTable[GUIColorExt_gray19] = GUICreateColorSlot(GUIState, GUIColorHex("#303030"), "gray19");
	GUIState->ColorTable[GUIColorExt_gray2] = GUICreateColorSlot(GUIState, GUIColorHex("#050505"), "gray2");
	GUIState->ColorTable[GUIColorExt_gray20] = GUICreateColorSlot(GUIState, GUIColorHex("#333333"), "gray20");
	GUIState->ColorTable[GUIColorExt_gray21] = GUICreateColorSlot(GUIState, GUIColorHex("#363636"), "gray21");
	GUIState->ColorTable[GUIColorExt_gray22] = GUICreateColorSlot(GUIState, GUIColorHex("#383838"), "gray22");
	GUIState->ColorTable[GUIColorExt_gray23] = GUICreateColorSlot(GUIState, GUIColorHex("#3b3b3b"), "gray23");
	GUIState->ColorTable[GUIColorExt_gray24] = GUICreateColorSlot(GUIState, GUIColorHex("#3d3d3d"), "gray24");
	GUIState->ColorTable[GUIColorExt_gray25] = GUICreateColorSlot(GUIState, GUIColorHex("#404040"), "gray25");
	GUIState->ColorTable[GUIColorExt_gray26] = GUICreateColorSlot(GUIState, GUIColorHex("#424242"), "gray26");
	GUIState->ColorTable[GUIColorExt_gray27] = GUICreateColorSlot(GUIState, GUIColorHex("#454545"), "gray27");
	GUIState->ColorTable[GUIColorExt_gray28] = GUICreateColorSlot(GUIState, GUIColorHex("#474747"), "gray28");
	GUIState->ColorTable[GUIColorExt_gray29] = GUICreateColorSlot(GUIState, GUIColorHex("#4a4a4a"), "gray29");
	GUIState->ColorTable[GUIColorExt_gray3] = GUICreateColorSlot(GUIState, GUIColorHex("#080808"), "gray3");
	GUIState->ColorTable[GUIColorExt_gray30] = GUICreateColorSlot(GUIState, GUIColorHex("#4d4d4d"), "gray30");
	GUIState->ColorTable[GUIColorExt_gray31] = GUICreateColorSlot(GUIState, GUIColorHex("#4f4f4f"), "gray31");
	GUIState->ColorTable[GUIColorExt_gray32] = GUICreateColorSlot(GUIState, GUIColorHex("#525252"), "gray32");
	GUIState->ColorTable[GUIColorExt_gray33] = GUICreateColorSlot(GUIState, GUIColorHex("#545454"), "gray33");
	GUIState->ColorTable[GUIColorExt_gray34] = GUICreateColorSlot(GUIState, GUIColorHex("#575757"), "gray34");
	GUIState->ColorTable[GUIColorExt_gray35] = GUICreateColorSlot(GUIState, GUIColorHex("#595959"), "gray35");
	GUIState->ColorTable[GUIColorExt_gray36] = GUICreateColorSlot(GUIState, GUIColorHex("#5c5c5c"), "gray36");
	GUIState->ColorTable[GUIColorExt_gray37] = GUICreateColorSlot(GUIState, GUIColorHex("#5e5e5e"), "gray37");
	GUIState->ColorTable[GUIColorExt_gray38] = GUICreateColorSlot(GUIState, GUIColorHex("#616161"), "gray38");
	GUIState->ColorTable[GUIColorExt_gray39] = GUICreateColorSlot(GUIState, GUIColorHex("#636363"), "gray39");
	GUIState->ColorTable[GUIColorExt_gray4] = GUICreateColorSlot(GUIState, GUIColorHex("#0a0a0a"), "gray4");
	GUIState->ColorTable[GUIColorExt_gray40] = GUICreateColorSlot(GUIState, GUIColorHex("#666666"), "gray40");
	GUIState->ColorTable[GUIColorExt_gray41] = GUICreateColorSlot(GUIState, GUIColorHex("#696969"), "gray41");
	GUIState->ColorTable[GUIColorExt_gray42] = GUICreateColorSlot(GUIState, GUIColorHex("#6b6b6b"), "gray42");
	GUIState->ColorTable[GUIColorExt_gray43] = GUICreateColorSlot(GUIState, GUIColorHex("#6e6e6e"), "gray43");
	GUIState->ColorTable[GUIColorExt_gray44] = GUICreateColorSlot(GUIState, GUIColorHex("#707070"), "gray44");
	GUIState->ColorTable[GUIColorExt_gray45] = GUICreateColorSlot(GUIState, GUIColorHex("#737373"), "gray45");
	GUIState->ColorTable[GUIColorExt_gray46] = GUICreateColorSlot(GUIState, GUIColorHex("#757575"), "gray46");
	GUIState->ColorTable[GUIColorExt_gray47] = GUICreateColorSlot(GUIState, GUIColorHex("#787878"), "gray47");
	GUIState->ColorTable[GUIColorExt_gray48] = GUICreateColorSlot(GUIState, GUIColorHex("#7a7a7a"), "gray48");
	GUIState->ColorTable[GUIColorExt_gray49] = GUICreateColorSlot(GUIState, GUIColorHex("#7d7d7d"), "gray49");
	GUIState->ColorTable[GUIColorExt_gray5] = GUICreateColorSlot(GUIState, GUIColorHex("#0d0d0d"), "gray5");
	GUIState->ColorTable[GUIColorExt_gray50] = GUICreateColorSlot(GUIState, GUIColorHex("#7f7f7f"), "gray50");
	GUIState->ColorTable[GUIColorExt_gray51] = GUICreateColorSlot(GUIState, GUIColorHex("#828282"), "gray51");
	GUIState->ColorTable[GUIColorExt_gray52] = GUICreateColorSlot(GUIState, GUIColorHex("#858585"), "gray52");
	GUIState->ColorTable[GUIColorExt_gray53] = GUICreateColorSlot(GUIState, GUIColorHex("#878787"), "gray53");
	GUIState->ColorTable[GUIColorExt_gray54] = GUICreateColorSlot(GUIState, GUIColorHex("#8a8a8a"), "gray54");
	GUIState->ColorTable[GUIColorExt_gray55] = GUICreateColorSlot(GUIState, GUIColorHex("#8c8c8c"), "gray55");
	GUIState->ColorTable[GUIColorExt_gray56] = GUICreateColorSlot(GUIState, GUIColorHex("#8f8f8f"), "gray56");
	GUIState->ColorTable[GUIColorExt_gray57] = GUICreateColorSlot(GUIState, GUIColorHex("#919191"), "gray57");
	GUIState->ColorTable[GUIColorExt_gray58] = GUICreateColorSlot(GUIState, GUIColorHex("#949494"), "gray58");
	GUIState->ColorTable[GUIColorExt_gray59] = GUICreateColorSlot(GUIState, GUIColorHex("#969696"), "gray59");
	GUIState->ColorTable[GUIColorExt_gray6] = GUICreateColorSlot(GUIState, GUIColorHex("#0f0f0f"), "gray6");
	GUIState->ColorTable[GUIColorExt_gray60] = GUICreateColorSlot(GUIState, GUIColorHex("#999999"), "gray60");
	GUIState->ColorTable[GUIColorExt_gray61] = GUICreateColorSlot(GUIState, GUIColorHex("#9c9c9c"), "gray61");
	GUIState->ColorTable[GUIColorExt_gray62] = GUICreateColorSlot(GUIState, GUIColorHex("#9e9e9e"), "gray62");
	GUIState->ColorTable[GUIColorExt_gray63] = GUICreateColorSlot(GUIState, GUIColorHex("#a1a1a1"), "gray63");
	GUIState->ColorTable[GUIColorExt_gray64] = GUICreateColorSlot(GUIState, GUIColorHex("#a3a3a3"), "gray64");
	GUIState->ColorTable[GUIColorExt_gray65] = GUICreateColorSlot(GUIState, GUIColorHex("#a6a6a6"), "gray65");
	GUIState->ColorTable[GUIColorExt_gray66] = GUICreateColorSlot(GUIState, GUIColorHex("#a8a8a8"), "gray66");
	GUIState->ColorTable[GUIColorExt_gray67] = GUICreateColorSlot(GUIState, GUIColorHex("#ababab"), "gray67");
	GUIState->ColorTable[GUIColorExt_gray68] = GUICreateColorSlot(GUIState, GUIColorHex("#adadad"), "gray68");
	GUIState->ColorTable[GUIColorExt_gray69] = GUICreateColorSlot(GUIState, GUIColorHex("#b0b0b0"), "gray69");
	GUIState->ColorTable[GUIColorExt_gray7] = GUICreateColorSlot(GUIState, GUIColorHex("#121212"), "gray7");
	GUIState->ColorTable[GUIColorExt_gray70] = GUICreateColorSlot(GUIState, GUIColorHex("#b3b3b3"), "gray70");
	GUIState->ColorTable[GUIColorExt_gray71] = GUICreateColorSlot(GUIState, GUIColorHex("#b5b5b5"), "gray71");
	GUIState->ColorTable[GUIColorExt_gray72] = GUICreateColorSlot(GUIState, GUIColorHex("#b8b8b8"), "gray72");
	GUIState->ColorTable[GUIColorExt_gray73] = GUICreateColorSlot(GUIState, GUIColorHex("#bababa"), "gray73");
	GUIState->ColorTable[GUIColorExt_gray74] = GUICreateColorSlot(GUIState, GUIColorHex("#bdbdbd"), "gray74");
	GUIState->ColorTable[GUIColorExt_gray75] = GUICreateColorSlot(GUIState, GUIColorHex("#bfbfbf"), "gray75");
	GUIState->ColorTable[GUIColorExt_gray76] = GUICreateColorSlot(GUIState, GUIColorHex("#c2c2c2"), "gray76");
	GUIState->ColorTable[GUIColorExt_gray77] = GUICreateColorSlot(GUIState, GUIColorHex("#c4c4c4"), "gray77");
	GUIState->ColorTable[GUIColorExt_gray78] = GUICreateColorSlot(GUIState, GUIColorHex("#c7c7c7"), "gray78");
	GUIState->ColorTable[GUIColorExt_gray79] = GUICreateColorSlot(GUIState, GUIColorHex("#c9c9c9"), "gray79");
	GUIState->ColorTable[GUIColorExt_gray8] = GUICreateColorSlot(GUIState, GUIColorHex("#141414"), "gray8");
	GUIState->ColorTable[GUIColorExt_gray80] = GUICreateColorSlot(GUIState, GUIColorHex("#cccccc"), "gray80");
	GUIState->ColorTable[GUIColorExt_gray81] = GUICreateColorSlot(GUIState, GUIColorHex("#cfcfcf"), "gray81");
	GUIState->ColorTable[GUIColorExt_gray82] = GUICreateColorSlot(GUIState, GUIColorHex("#d1d1d1"), "gray82");
	GUIState->ColorTable[GUIColorExt_gray83] = GUICreateColorSlot(GUIState, GUIColorHex("#d4d4d4"), "gray83");
	GUIState->ColorTable[GUIColorExt_gray84] = GUICreateColorSlot(GUIState, GUIColorHex("#d6d6d6"), "gray84");
	GUIState->ColorTable[GUIColorExt_gray85] = GUICreateColorSlot(GUIState, GUIColorHex("#d9d9d9"), "gray85");
	GUIState->ColorTable[GUIColorExt_gray86] = GUICreateColorSlot(GUIState, GUIColorHex("#dbdbdb"), "gray86");
	GUIState->ColorTable[GUIColorExt_gray87] = GUICreateColorSlot(GUIState, GUIColorHex("#dedede"), "gray87");
	GUIState->ColorTable[GUIColorExt_gray88] = GUICreateColorSlot(GUIState, GUIColorHex("#e0e0e0"), "gray88");
	GUIState->ColorTable[GUIColorExt_gray89] = GUICreateColorSlot(GUIState, GUIColorHex("#e3e3e3"), "gray89");
	GUIState->ColorTable[GUIColorExt_gray9] = GUICreateColorSlot(GUIState, GUIColorHex("#171717"), "gray9");
	GUIState->ColorTable[GUIColorExt_gray90] = GUICreateColorSlot(GUIState, GUIColorHex("#e5e5e5"), "gray90");
	GUIState->ColorTable[GUIColorExt_gray91] = GUICreateColorSlot(GUIState, GUIColorHex("#e8e8e8"), "gray91");
	GUIState->ColorTable[GUIColorExt_gray92] = GUICreateColorSlot(GUIState, GUIColorHex("#ebebeb"), "gray92");
	GUIState->ColorTable[GUIColorExt_gray93] = GUICreateColorSlot(GUIState, GUIColorHex("#ededed"), "gray93");
	GUIState->ColorTable[GUIColorExt_gray94] = GUICreateColorSlot(GUIState, GUIColorHex("#f0f0f0"), "gray94");
	GUIState->ColorTable[GUIColorExt_gray95] = GUICreateColorSlot(GUIState, GUIColorHex("#f2f2f2"), "gray95");
	GUIState->ColorTable[GUIColorExt_gray97] = GUICreateColorSlot(GUIState, GUIColorHex("#f7f7f7"), "gray97");
	GUIState->ColorTable[GUIColorExt_gray98] = GUICreateColorSlot(GUIState, GUIColorHex("#fafafa"), "gray98");
	GUIState->ColorTable[GUIColorExt_gray99] = GUICreateColorSlot(GUIState, GUIColorHex("#fcfcfc"), "gray99");
	GUIState->ColorTable[GUIColorExt_green1] = GUICreateColorSlot(GUIState, GUIColorHex("#00ff00"), "green1");
	GUIState->ColorTable[GUIColorExt_green2] = GUICreateColorSlot(GUIState, GUIColorHex("#00ee00"), "green2");
	GUIState->ColorTable[GUIColorExt_green3] = GUICreateColorSlot(GUIState, GUIColorHex("#00cd00"), "green3");
	GUIState->ColorTable[GUIColorExt_green4] = GUICreateColorSlot(GUIState, GUIColorHex("#008b00"), "green4");
	GUIState->ColorTable[GUIColorExt_GreenYellow] = GUICreateColorSlot(GUIState, GUIColorHex("#adff2f"), "GreenYellow");
	GUIState->ColorTable[GUIColorExt_honeydew1] = GUICreateColorSlot(GUIState, GUIColorHex("#f0fff0"), "honeydew1");
	GUIState->ColorTable[GUIColorExt_honeydew2] = GUICreateColorSlot(GUIState, GUIColorHex("#e0eee0"), "honeydew2");
	GUIState->ColorTable[GUIColorExt_honeydew3] = GUICreateColorSlot(GUIState, GUIColorHex("#c1cdc1"), "honeydew3");
	GUIState->ColorTable[GUIColorExt_honeydew4] = GUICreateColorSlot(GUIState, GUIColorHex("#838b83"), "honeydew4");
	GUIState->ColorTable[GUIColorExt_HotPink] = GUICreateColorSlot(GUIState, GUIColorHex("#ff69b4"), "HotPink");
	GUIState->ColorTable[GUIColorExt_HotPink1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff6eb4"), "HotPink1");
	GUIState->ColorTable[GUIColorExt_HotPink2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee6aa7"), "HotPink2");
	GUIState->ColorTable[GUIColorExt_HotPink3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd6090"), "HotPink3");
	GUIState->ColorTable[GUIColorExt_HotPink4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b3a62"), "HotPink4");
	GUIState->ColorTable[GUIColorExt_IndianRed] = GUICreateColorSlot(GUIState, GUIColorHex("#cd5c5c"), "IndianRed");
	GUIState->ColorTable[GUIColorExt_IndianRed1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff6a6a"), "IndianRed1");
	GUIState->ColorTable[GUIColorExt_IndianRed2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee6363"), "IndianRed2");
	GUIState->ColorTable[GUIColorExt_IndianRed3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd5555"), "IndianRed3");
	GUIState->ColorTable[GUIColorExt_IndianRed4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b3a3a"), "IndianRed4");
	GUIState->ColorTable[GUIColorExt_ivory1] = GUICreateColorSlot(GUIState, GUIColorHex("#fffff0"), "ivory1");
	GUIState->ColorTable[GUIColorExt_ivory2] = GUICreateColorSlot(GUIState, GUIColorHex("#eeeee0"), "ivory2");
	GUIState->ColorTable[GUIColorExt_ivory3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdcdc1"), "ivory3");
	GUIState->ColorTable[GUIColorExt_ivory4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8b83"), "ivory4");
	GUIState->ColorTable[GUIColorExt_khaki] = GUICreateColorSlot(GUIState, GUIColorHex("#f0e68c"), "khaki");
	GUIState->ColorTable[GUIColorExt_khaki1] = GUICreateColorSlot(GUIState, GUIColorHex("#fff68f"), "khaki1");
	GUIState->ColorTable[GUIColorExt_khaki2] = GUICreateColorSlot(GUIState, GUIColorHex("#eee685"), "khaki2");
	GUIState->ColorTable[GUIColorExt_khaki3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdc673"), "khaki3");
	GUIState->ColorTable[GUIColorExt_khaki4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b864e"), "khaki4");
	GUIState->ColorTable[GUIColorExt_lavender] = GUICreateColorSlot(GUIState, GUIColorHex("#e6e6fa"), "lavender");
	GUIState->ColorTable[GUIColorExt_LavenderBlush1] = GUICreateColorSlot(GUIState, GUIColorHex("#fff0f5"), "LavenderBlush1");
	GUIState->ColorTable[GUIColorExt_LavenderBlush2] = GUICreateColorSlot(GUIState, GUIColorHex("#eee0e5"), "LavenderBlush2");
	GUIState->ColorTable[GUIColorExt_LavenderBlush3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdc1c5"), "LavenderBlush3");
	GUIState->ColorTable[GUIColorExt_LavenderBlush4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8386"), "LavenderBlush4");
	GUIState->ColorTable[GUIColorExt_LawnGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#7cfc00"), "LawnGreen");
	GUIState->ColorTable[GUIColorExt_LemonChiffon1] = GUICreateColorSlot(GUIState, GUIColorHex("#fffacd"), "LemonChiffon1");
	GUIState->ColorTable[GUIColorExt_LemonChiffon2] = GUICreateColorSlot(GUIState, GUIColorHex("#eee9bf"), "LemonChiffon2");
	GUIState->ColorTable[GUIColorExt_LemonChiffon3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdc9a5"), "LemonChiffon3");
	GUIState->ColorTable[GUIColorExt_LemonChiffon4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8970"), "LemonChiffon4");
	GUIState->ColorTable[GUIColorExt_light] = GUICreateColorSlot(GUIState, GUIColorHex("#eedd82"), "light");
	GUIState->ColorTable[GUIColorExt_LightBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#add8e6"), "LightBlue");
	GUIState->ColorTable[GUIColorExt_LightBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#bfefff"), "LightBlue1");
	GUIState->ColorTable[GUIColorExt_LightBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#b2dfee"), "LightBlue2");
	GUIState->ColorTable[GUIColorExt_LightBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#9ac0cd"), "LightBlue3");
	GUIState->ColorTable[GUIColorExt_LightBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#68838b"), "LightBlue4");
	GUIState->ColorTable[GUIColorExt_LightCoral] = GUICreateColorSlot(GUIState, GUIColorHex("#f08080"), "LightCoral");
	GUIState->ColorTable[GUIColorExt_LightCyan1] = GUICreateColorSlot(GUIState, GUIColorHex("#e0ffff"), "LightCyan1");
	GUIState->ColorTable[GUIColorExt_LightCyan2] = GUICreateColorSlot(GUIState, GUIColorHex("#d1eeee"), "LightCyan2");
	GUIState->ColorTable[GUIColorExt_LightCyan3] = GUICreateColorSlot(GUIState, GUIColorHex("#b4cdcd"), "LightCyan3");
	GUIState->ColorTable[GUIColorExt_LightCyan4] = GUICreateColorSlot(GUIState, GUIColorHex("#7a8b8b"), "LightCyan4");
	GUIState->ColorTable[GUIColorExt_LightGoldenrod1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffec8b"), "LightGoldenrod1");
	GUIState->ColorTable[GUIColorExt_LightGoldenrod2] = GUICreateColorSlot(GUIState, GUIColorHex("#eedc82"), "LightGoldenrod2");
	GUIState->ColorTable[GUIColorExt_LightGoldenrod3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdbe70"), "LightGoldenrod3");
	GUIState->ColorTable[GUIColorExt_LightGoldenrod4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b814c"), "LightGoldenrod4");
	GUIState->ColorTable[GUIColorExt_LightGoldenrodYellow] = GUICreateColorSlot(GUIState, GUIColorHex("#fafad2"), "LightGoldenrodYellow");
	GUIState->ColorTable[GUIColorExt_LightGray] = GUICreateColorSlot(GUIState, GUIColorHex("#d3d3d3"), "LightGray");
	GUIState->ColorTable[GUIColorExt_LightPink] = GUICreateColorSlot(GUIState, GUIColorHex("#ffb6c1"), "LightPink");
	GUIState->ColorTable[GUIColorExt_LightPink1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffaeb9"), "LightPink1");
	GUIState->ColorTable[GUIColorExt_LightPink2] = GUICreateColorSlot(GUIState, GUIColorHex("#eea2ad"), "LightPink2");
	GUIState->ColorTable[GUIColorExt_LightPink3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd8c95"), "LightPink3");
	GUIState->ColorTable[GUIColorExt_LightPink4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b5f65"), "LightPink4");
	GUIState->ColorTable[GUIColorExt_LightSalmon1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffa07a"), "LightSalmon1");
	GUIState->ColorTable[GUIColorExt_LightSalmon2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee9572"), "LightSalmon2");
	GUIState->ColorTable[GUIColorExt_LightSalmon3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd8162"), "LightSalmon3");
	GUIState->ColorTable[GUIColorExt_LightSalmon4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b5742"), "LightSalmon4");
	GUIState->ColorTable[GUIColorExt_LightSeaGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#20b2aa"), "LightSeaGreen");
	GUIState->ColorTable[GUIColorExt_LightSkyBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#87cefa"), "LightSkyBlue");
	GUIState->ColorTable[GUIColorExt_LightSkyBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#b0e2ff"), "LightSkyBlue1");
	GUIState->ColorTable[GUIColorExt_LightSkyBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#a4d3ee"), "LightSkyBlue2");
	GUIState->ColorTable[GUIColorExt_LightSkyBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#8db6cd"), "LightSkyBlue3");
	GUIState->ColorTable[GUIColorExt_LightSkyBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#607b8b"), "LightSkyBlue4");
	GUIState->ColorTable[GUIColorExt_LightSlateBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#8470ff"), "LightSlateBlue");
	GUIState->ColorTable[GUIColorExt_LightSlateGray] = GUICreateColorSlot(GUIState, GUIColorHex("#778899"), "LightSlateGray");
	GUIState->ColorTable[GUIColorExt_LightSteelBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#b0c4de"), "LightSteelBlue");
	GUIState->ColorTable[GUIColorExt_LightSteelBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#cae1ff"), "LightSteelBlue1");
	GUIState->ColorTable[GUIColorExt_LightSteelBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#bcd2ee"), "LightSteelBlue2");
	GUIState->ColorTable[GUIColorExt_LightSteelBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#a2b5cd"), "LightSteelBlue3");
	GUIState->ColorTable[GUIColorExt_LightSteelBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#6e7b8b"), "LightSteelBlue4");
	GUIState->ColorTable[GUIColorExt_LightYellow1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffffe0"), "LightYellow1");
	GUIState->ColorTable[GUIColorExt_LightYellow2] = GUICreateColorSlot(GUIState, GUIColorHex("#eeeed1"), "LightYellow2");
	GUIState->ColorTable[GUIColorExt_LightYellow3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdcdb4"), "LightYellow3");
	GUIState->ColorTable[GUIColorExt_LightYellow4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8b7a"), "LightYellow4");
	GUIState->ColorTable[GUIColorExt_LimeGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#32cd32"), "LimeGreen");
	GUIState->ColorTable[GUIColorExt_linen] = GUICreateColorSlot(GUIState, GUIColorHex("#faf0e6"), "linen");
	GUIState->ColorTable[GUIColorExt_magenta] = GUICreateColorSlot(GUIState, GUIColorHex("#ff00ff"), "magenta");
	GUIState->ColorTable[GUIColorExt_magenta2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee00ee"), "magenta2");
	GUIState->ColorTable[GUIColorExt_magenta3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd00cd"), "magenta3");
	GUIState->ColorTable[GUIColorExt_magenta4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b008b"), "magenta4");
	GUIState->ColorTable[GUIColorExt_maroon] = GUICreateColorSlot(GUIState, GUIColorHex("#b03060"), "maroon");
	GUIState->ColorTable[GUIColorExt_maroon1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff34b3"), "maroon1");
	GUIState->ColorTable[GUIColorExt_maroon2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee30a7"), "maroon2");
	GUIState->ColorTable[GUIColorExt_maroon3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd2990"), "maroon3");
	GUIState->ColorTable[GUIColorExt_maroon4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b1c62"), "maroon4");
	GUIState->ColorTable[GUIColorExt_medium] = GUICreateColorSlot(GUIState, GUIColorHex("#66cdaa"), "medium");
	GUIState->ColorTable[GUIColorExt_MediumAquamarine] = GUICreateColorSlot(GUIState, GUIColorHex("#66cdaa"), "MediumAquamarine");
	GUIState->ColorTable[GUIColorExt_MediumBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#0000cd"), "MediumBlue");
	GUIState->ColorTable[GUIColorExt_MediumOrchid] = GUICreateColorSlot(GUIState, GUIColorHex("#ba55d3"), "MediumOrchid");
	GUIState->ColorTable[GUIColorExt_MediumOrchid1] = GUICreateColorSlot(GUIState, GUIColorHex("#e066ff"), "MediumOrchid1");
	GUIState->ColorTable[GUIColorExt_MediumOrchid2] = GUICreateColorSlot(GUIState, GUIColorHex("#d15fee"), "MediumOrchid2");
	GUIState->ColorTable[GUIColorExt_MediumOrchid3] = GUICreateColorSlot(GUIState, GUIColorHex("#b452cd"), "MediumOrchid3");
	GUIState->ColorTable[GUIColorExt_MediumOrchid4] = GUICreateColorSlot(GUIState, GUIColorHex("#7a378b"), "MediumOrchid4");
	GUIState->ColorTable[GUIColorExt_MediumPurple] = GUICreateColorSlot(GUIState, GUIColorHex("#9370db"), "MediumPurple");
	GUIState->ColorTable[GUIColorExt_MediumPurple1] = GUICreateColorSlot(GUIState, GUIColorHex("#ab82ff"), "MediumPurple1");
	GUIState->ColorTable[GUIColorExt_MediumPurple2] = GUICreateColorSlot(GUIState, GUIColorHex("#9f79ee"), "MediumPurple2");
	GUIState->ColorTable[GUIColorExt_MediumPurple3] = GUICreateColorSlot(GUIState, GUIColorHex("#8968cd"), "MediumPurple3");
	GUIState->ColorTable[GUIColorExt_MediumPurple4] = GUICreateColorSlot(GUIState, GUIColorHex("#5d478b"), "MediumPurple4");
	GUIState->ColorTable[GUIColorExt_MediumSeaGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#3cb371"), "MediumSeaGreen");
	GUIState->ColorTable[GUIColorExt_MediumSlateBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#7b68ee"), "MediumSlateBlue");
	GUIState->ColorTable[GUIColorExt_MediumSpringGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#00fa9a"), "MediumSpringGreen");
	GUIState->ColorTable[GUIColorExt_MediumTurquoise] = GUICreateColorSlot(GUIState, GUIColorHex("#48d1cc"), "MediumTurquoise");
	GUIState->ColorTable[GUIColorExt_MediumVioletRed] = GUICreateColorSlot(GUIState, GUIColorHex("#c71585"), "MediumVioletRed");
	GUIState->ColorTable[GUIColorExt_MidnightBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#191970"), "MidnightBlue");
	GUIState->ColorTable[GUIColorExt_MintCream] = GUICreateColorSlot(GUIState, GUIColorHex("#f5fffa"), "MintCream");
	GUIState->ColorTable[GUIColorExt_MistyRose1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffe4e1"), "MistyRose1");
	GUIState->ColorTable[GUIColorExt_MistyRose2] = GUICreateColorSlot(GUIState, GUIColorHex("#eed5d2"), "MistyRose2");
	GUIState->ColorTable[GUIColorExt_MistyRose3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdb7b5"), "MistyRose3");
	GUIState->ColorTable[GUIColorExt_MistyRose4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b7d7b"), "MistyRose4");
	GUIState->ColorTable[GUIColorExt_moccasin] = GUICreateColorSlot(GUIState, GUIColorHex("#ffe4b5"), "moccasin");
	GUIState->ColorTable[GUIColorExt_NavajoWhite1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffdead"), "NavajoWhite1");
	GUIState->ColorTable[GUIColorExt_NavajoWhite2] = GUICreateColorSlot(GUIState, GUIColorHex("#eecfa1"), "NavajoWhite2");
	GUIState->ColorTable[GUIColorExt_NavajoWhite3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdb38b"), "NavajoWhite3");
	GUIState->ColorTable[GUIColorExt_NavajoWhite4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b795e"), "NavajoWhite4");
	GUIState->ColorTable[GUIColorExt_NavyBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#000080"), "NavyBlue");
	GUIState->ColorTable[GUIColorExt_OldLace] = GUICreateColorSlot(GUIState, GUIColorHex("#fdf5e6"), "OldLace");
	GUIState->ColorTable[GUIColorExt_OliveDrab] = GUICreateColorSlot(GUIState, GUIColorHex("#6b8e23"), "OliveDrab");
	GUIState->ColorTable[GUIColorExt_OliveDrab1] = GUICreateColorSlot(GUIState, GUIColorHex("#c0ff3e"), "OliveDrab1");
	GUIState->ColorTable[GUIColorExt_OliveDrab2] = GUICreateColorSlot(GUIState, GUIColorHex("#b3ee3a"), "OliveDrab2");
	GUIState->ColorTable[GUIColorExt_OliveDrab4] = GUICreateColorSlot(GUIState, GUIColorHex("#698b22"), "OliveDrab4");
	GUIState->ColorTable[GUIColorExt_orange1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffa500"), "orange1");
	GUIState->ColorTable[GUIColorExt_orange2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee9a00"), "orange2");
	GUIState->ColorTable[GUIColorExt_orange3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd8500"), "orange3");
	GUIState->ColorTable[GUIColorExt_orange4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b5a00"), "orange4");
	GUIState->ColorTable[GUIColorExt_OrangeRed1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff4500"), "OrangeRed1");
	GUIState->ColorTable[GUIColorExt_OrangeRed2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee4000"), "OrangeRed2");
	GUIState->ColorTable[GUIColorExt_OrangeRed3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd3700"), "OrangeRed3");
	GUIState->ColorTable[GUIColorExt_OrangeRed4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b2500"), "OrangeRed4");
	GUIState->ColorTable[GUIColorExt_orchid] = GUICreateColorSlot(GUIState, GUIColorHex("#da70d6"), "orchid");
	GUIState->ColorTable[GUIColorExt_orchid1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff83fa"), "orchid1");
	GUIState->ColorTable[GUIColorExt_orchid2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee7ae9"), "orchid2");
	GUIState->ColorTable[GUIColorExt_orchid3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd69c9"), "orchid3");
	GUIState->ColorTable[GUIColorExt_orchid4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b4789"), "orchid4");
	GUIState->ColorTable[GUIColorExt_pale] = GUICreateColorSlot(GUIState, GUIColorHex("#db7093"), "pale");
	GUIState->ColorTable[GUIColorExt_PaleGoldenrod] = GUICreateColorSlot(GUIState, GUIColorHex("#eee8aa"), "PaleGoldenrod");
	GUIState->ColorTable[GUIColorExt_PaleGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#98fb98"), "PaleGreen");
	GUIState->ColorTable[GUIColorExt_PaleGreen1] = GUICreateColorSlot(GUIState, GUIColorHex("#9aff9a"), "PaleGreen1");
	GUIState->ColorTable[GUIColorExt_PaleGreen2] = GUICreateColorSlot(GUIState, GUIColorHex("#90ee90"), "PaleGreen2");
	GUIState->ColorTable[GUIColorExt_PaleGreen3] = GUICreateColorSlot(GUIState, GUIColorHex("#7ccd7c"), "PaleGreen3");
	GUIState->ColorTable[GUIColorExt_PaleGreen4] = GUICreateColorSlot(GUIState, GUIColorHex("#548b54"), "PaleGreen4");
	GUIState->ColorTable[GUIColorExt_PaleTurquoise] = GUICreateColorSlot(GUIState, GUIColorHex("#afeeee"), "PaleTurquoise");
	GUIState->ColorTable[GUIColorExt_PaleTurquoise1] = GUICreateColorSlot(GUIState, GUIColorHex("#bbffff"), "PaleTurquoise1");
	GUIState->ColorTable[GUIColorExt_PaleTurquoise2] = GUICreateColorSlot(GUIState, GUIColorHex("#aeeeee"), "PaleTurquoise2");
	GUIState->ColorTable[GUIColorExt_PaleTurquoise3] = GUICreateColorSlot(GUIState, GUIColorHex("#96cdcd"), "PaleTurquoise3");
	GUIState->ColorTable[GUIColorExt_PaleTurquoise4] = GUICreateColorSlot(GUIState, GUIColorHex("#668b8b"), "PaleTurquoise4");
	GUIState->ColorTable[GUIColorExt_PaleVioletRed] = GUICreateColorSlot(GUIState, GUIColorHex("#db7093"), "PaleVioletRed");
	GUIState->ColorTable[GUIColorExt_PaleVioletRed1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff82ab"), "PaleVioletRed1");
	GUIState->ColorTable[GUIColorExt_PaleVioletRed2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee799f"), "PaleVioletRed2");
	GUIState->ColorTable[GUIColorExt_PaleVioletRed3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd6889"), "PaleVioletRed3");
	GUIState->ColorTable[GUIColorExt_PaleVioletRed4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b475d"), "PaleVioletRed4");
	GUIState->ColorTable[GUIColorExt_PapayaWhip] = GUICreateColorSlot(GUIState, GUIColorHex("#ffefd5"), "PapayaWhip");
	GUIState->ColorTable[GUIColorExt_PeachPuff1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffdab9"), "PeachPuff1");
	GUIState->ColorTable[GUIColorExt_PeachPuff2] = GUICreateColorSlot(GUIState, GUIColorHex("#eecbad"), "PeachPuff2");
	GUIState->ColorTable[GUIColorExt_PeachPuff3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdaf95"), "PeachPuff3");
	GUIState->ColorTable[GUIColorExt_PeachPuff4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b7765"), "PeachPuff4");
	GUIState->ColorTable[GUIColorExt_pink] = GUICreateColorSlot(GUIState, GUIColorHex("#ffc0cb"), "pink");
	GUIState->ColorTable[GUIColorExt_pink1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffb5c5"), "pink1");
	GUIState->ColorTable[GUIColorExt_pink2] = GUICreateColorSlot(GUIState, GUIColorHex("#eea9b8"), "pink2");
	GUIState->ColorTable[GUIColorExt_pink3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd919e"), "pink3");
	GUIState->ColorTable[GUIColorExt_pink4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b636c"), "pink4");
	GUIState->ColorTable[GUIColorExt_plum] = GUICreateColorSlot(GUIState, GUIColorHex("#dda0dd"), "plum");
	GUIState->ColorTable[GUIColorExt_plum1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffbbff"), "plum1");
	GUIState->ColorTable[GUIColorExt_plum2] = GUICreateColorSlot(GUIState, GUIColorHex("#eeaeee"), "plum2");
	GUIState->ColorTable[GUIColorExt_plum3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd96cd"), "plum3");
	GUIState->ColorTable[GUIColorExt_plum4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b668b"), "plum4");
	GUIState->ColorTable[GUIColorExt_PowderBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#b0e0e6"), "PowderBlue");
	GUIState->ColorTable[GUIColorExt_purple] = GUICreateColorSlot(GUIState, GUIColorHex("#a020f0"), "purple");
	GUIState->ColorTable[GUIColorExt_rebeccapurple] = GUICreateColorSlot(GUIState, GUIColorHex("#663399"), "rebeccapurple");
	GUIState->ColorTable[GUIColorExt_purple1] = GUICreateColorSlot(GUIState, GUIColorHex("#9b30ff"), "purple1");
	GUIState->ColorTable[GUIColorExt_purple2] = GUICreateColorSlot(GUIState, GUIColorHex("#912cee"), "purple2");
	GUIState->ColorTable[GUIColorExt_purple3] = GUICreateColorSlot(GUIState, GUIColorHex("#7d26cd"), "purple3");
	GUIState->ColorTable[GUIColorExt_purple4] = GUICreateColorSlot(GUIState, GUIColorHex("#551a8b"), "purple4");
	GUIState->ColorTable[GUIColorExt_red1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff0000"), "red1");
	GUIState->ColorTable[GUIColorExt_red2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee0000"), "red2");
	GUIState->ColorTable[GUIColorExt_red3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd0000"), "red3");
	GUIState->ColorTable[GUIColorExt_red4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b0000"), "red4");
	GUIState->ColorTable[GUIColorExt_RosyBrown] = GUICreateColorSlot(GUIState, GUIColorHex("#bc8f8f"), "RosyBrown");
	GUIState->ColorTable[GUIColorExt_RosyBrown1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffc1c1"), "RosyBrown1");
	GUIState->ColorTable[GUIColorExt_RosyBrown2] = GUICreateColorSlot(GUIState, GUIColorHex("#eeb4b4"), "RosyBrown2");
	GUIState->ColorTable[GUIColorExt_RosyBrown3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd9b9b"), "RosyBrown3");
	GUIState->ColorTable[GUIColorExt_RosyBrown4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b6969"), "RosyBrown4");
	GUIState->ColorTable[GUIColorExt_RoyalBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#4169e1"), "RoyalBlue");
	GUIState->ColorTable[GUIColorExt_RoyalBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#4876ff"), "RoyalBlue1");
	GUIState->ColorTable[GUIColorExt_RoyalBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#436eee"), "RoyalBlue2");
	GUIState->ColorTable[GUIColorExt_RoyalBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#3a5fcd"), "RoyalBlue3");
	GUIState->ColorTable[GUIColorExt_RoyalBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#27408b"), "RoyalBlue4");
	GUIState->ColorTable[GUIColorExt_SaddleBrown] = GUICreateColorSlot(GUIState, GUIColorHex("#8b4513"), "SaddleBrown");
	GUIState->ColorTable[GUIColorExt_salmon] = GUICreateColorSlot(GUIState, GUIColorHex("#fa8072"), "salmon");
	GUIState->ColorTable[GUIColorExt_salmon1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff8c69"), "salmon1");
	GUIState->ColorTable[GUIColorExt_salmon2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee8262"), "salmon2");
	GUIState->ColorTable[GUIColorExt_salmon3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd7054"), "salmon3");
	GUIState->ColorTable[GUIColorExt_salmon4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b4c39"), "salmon4");
	GUIState->ColorTable[GUIColorExt_SandyBrown] = GUICreateColorSlot(GUIState, GUIColorHex("#f4a460"), "SandyBrown");
	GUIState->ColorTable[GUIColorExt_SeaGreen1] = GUICreateColorSlot(GUIState, GUIColorHex("#54ff9f"), "SeaGreen1");
	GUIState->ColorTable[GUIColorExt_SeaGreen2] = GUICreateColorSlot(GUIState, GUIColorHex("#4eee94"), "SeaGreen2");
	GUIState->ColorTable[GUIColorExt_SeaGreen3] = GUICreateColorSlot(GUIState, GUIColorHex("#43cd80"), "SeaGreen3");
	GUIState->ColorTable[GUIColorExt_SeaGreen4] = GUICreateColorSlot(GUIState, GUIColorHex("#2e8b57"), "SeaGreen4");
	GUIState->ColorTable[GUIColorExt_seashell1] = GUICreateColorSlot(GUIState, GUIColorHex("#fff5ee"), "seashell1");
	GUIState->ColorTable[GUIColorExt_seashell2] = GUICreateColorSlot(GUIState, GUIColorHex("#eee5de"), "seashell2");
	GUIState->ColorTable[GUIColorExt_seashell3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdc5bf"), "seashell3");
	GUIState->ColorTable[GUIColorExt_seashell4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8682"), "seashell4");
	GUIState->ColorTable[GUIColorExt_sienna] = GUICreateColorSlot(GUIState, GUIColorHex("#a0522d"), "sienna");
	GUIState->ColorTable[GUIColorExt_sienna1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff8247"), "sienna1");
	GUIState->ColorTable[GUIColorExt_sienna2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee7942"), "sienna2");
	GUIState->ColorTable[GUIColorExt_sienna3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd6839"), "sienna3");
	GUIState->ColorTable[GUIColorExt_sienna4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b4726"), "sienna4");
	GUIState->ColorTable[GUIColorExt_SkyBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#87ceeb"), "SkyBlue");
	GUIState->ColorTable[GUIColorExt_SkyBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#87ceff"), "SkyBlue1");
	GUIState->ColorTable[GUIColorExt_SkyBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#7ec0ee"), "SkyBlue2");
	GUIState->ColorTable[GUIColorExt_SkyBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#6ca6cd"), "SkyBlue3");
	GUIState->ColorTable[GUIColorExt_SkyBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#4a708b"), "SkyBlue4");
	GUIState->ColorTable[GUIColorExt_SlateBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#6a5acd"), "SlateBlue");
	GUIState->ColorTable[GUIColorExt_SlateBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#836fff"), "SlateBlue1");
	GUIState->ColorTable[GUIColorExt_SlateBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#7a67ee"), "SlateBlue2");
	GUIState->ColorTable[GUIColorExt_SlateBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#6959cd"), "SlateBlue3");
	GUIState->ColorTable[GUIColorExt_SlateBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#473c8b"), "SlateBlue4");
	GUIState->ColorTable[GUIColorExt_SlateGray] = GUICreateColorSlot(GUIState, GUIColorHex("#708090"), "SlateGray");
	GUIState->ColorTable[GUIColorExt_SlateGray1] = GUICreateColorSlot(GUIState, GUIColorHex("#c6e2ff"), "SlateGray1");
	GUIState->ColorTable[GUIColorExt_SlateGray2] = GUICreateColorSlot(GUIState, GUIColorHex("#b9d3ee"), "SlateGray2");
	GUIState->ColorTable[GUIColorExt_SlateGray3] = GUICreateColorSlot(GUIState, GUIColorHex("#9fb6cd"), "SlateGray3");
	GUIState->ColorTable[GUIColorExt_SlateGray4] = GUICreateColorSlot(GUIState, GUIColorHex("#6c7b8b"), "SlateGray4");
	GUIState->ColorTable[GUIColorExt_snow1] = GUICreateColorSlot(GUIState, GUIColorHex("#fffafa"), "snow1");
	GUIState->ColorTable[GUIColorExt_snow2] = GUICreateColorSlot(GUIState, GUIColorHex("#eee9e9"), "snow2");
	GUIState->ColorTable[GUIColorExt_snow3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdc9c9"), "snow3");
	GUIState->ColorTable[GUIColorExt_snow4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8989"), "snow4");
	GUIState->ColorTable[GUIColorExt_SpringGreen1] = GUICreateColorSlot(GUIState, GUIColorHex("#00ff7f"), "SpringGreen1");
	GUIState->ColorTable[GUIColorExt_SpringGreen2] = GUICreateColorSlot(GUIState, GUIColorHex("#00ee76"), "SpringGreen2");
	GUIState->ColorTable[GUIColorExt_SpringGreen3] = GUICreateColorSlot(GUIState, GUIColorHex("#00cd66"), "SpringGreen3");
	GUIState->ColorTable[GUIColorExt_SpringGreen4] = GUICreateColorSlot(GUIState, GUIColorHex("#008b45"), "SpringGreen4");
	GUIState->ColorTable[GUIColorExt_SteelBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#4682b4"), "SteelBlue");
	GUIState->ColorTable[GUIColorExt_SteelBlue1] = GUICreateColorSlot(GUIState, GUIColorHex("#63b8ff"), "SteelBlue1");
	GUIState->ColorTable[GUIColorExt_SteelBlue2] = GUICreateColorSlot(GUIState, GUIColorHex("#5cacee"), "SteelBlue2");
	GUIState->ColorTable[GUIColorExt_SteelBlue3] = GUICreateColorSlot(GUIState, GUIColorHex("#4f94cd"), "SteelBlue3");
	GUIState->ColorTable[GUIColorExt_SteelBlue4] = GUICreateColorSlot(GUIState, GUIColorHex("#36648b"), "SteelBlue4");
	GUIState->ColorTable[GUIColorExt_tan] = GUICreateColorSlot(GUIState, GUIColorHex("#d2b48c"), "tan");
	GUIState->ColorTable[GUIColorExt_tan1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffa54f"), "tan1");
	GUIState->ColorTable[GUIColorExt_tan2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee9a49"), "tan2");
	GUIState->ColorTable[GUIColorExt_tan3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd853f"), "tan3");
	GUIState->ColorTable[GUIColorExt_tan4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b5a2b"), "tan4");
	GUIState->ColorTable[GUIColorExt_thistle] = GUICreateColorSlot(GUIState, GUIColorHex("#d8bfd8"), "thistle");
	GUIState->ColorTable[GUIColorExt_thistle1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffe1ff"), "thistle1");
	GUIState->ColorTable[GUIColorExt_thistle2] = GUICreateColorSlot(GUIState, GUIColorHex("#eed2ee"), "thistle2");
	GUIState->ColorTable[GUIColorExt_thistle3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdb5cd"), "thistle3");
	GUIState->ColorTable[GUIColorExt_thistle4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b7b8b"), "thistle4");
	GUIState->ColorTable[GUIColorExt_tomato1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff6347"), "tomato1");
	GUIState->ColorTable[GUIColorExt_tomato2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee5c42"), "tomato2");
	GUIState->ColorTable[GUIColorExt_tomato3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd4f39"), "tomato3");
	GUIState->ColorTable[GUIColorExt_tomato4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b3626"), "tomato4");
	GUIState->ColorTable[GUIColorExt_turquoise] = GUICreateColorSlot(GUIState, GUIColorHex("#40e0d0"), "turquoise");
	GUIState->ColorTable[GUIColorExt_turquoise1] = GUICreateColorSlot(GUIState, GUIColorHex("#00f5ff"), "turquoise1");
	GUIState->ColorTable[GUIColorExt_turquoise2] = GUICreateColorSlot(GUIState, GUIColorHex("#00e5ee"), "turquoise2");
	GUIState->ColorTable[GUIColorExt_turquoise3] = GUICreateColorSlot(GUIState, GUIColorHex("#00c5cd"), "turquoise3");
	GUIState->ColorTable[GUIColorExt_turquoise4] = GUICreateColorSlot(GUIState, GUIColorHex("#00868b"), "turquoise4");
	GUIState->ColorTable[GUIColorExt_violet] = GUICreateColorSlot(GUIState, GUIColorHex("#ee82ee"), "violet");
	GUIState->ColorTable[GUIColorExt_VioletRed] = GUICreateColorSlot(GUIState, GUIColorHex("#d02090"), "VioletRed");
	GUIState->ColorTable[GUIColorExt_VioletRed1] = GUICreateColorSlot(GUIState, GUIColorHex("#ff3e96"), "VioletRed1");
	GUIState->ColorTable[GUIColorExt_VioletRed2] = GUICreateColorSlot(GUIState, GUIColorHex("#ee3a8c"), "VioletRed2");
	GUIState->ColorTable[GUIColorExt_VioletRed3] = GUICreateColorSlot(GUIState, GUIColorHex("#cd3278"), "VioletRed3");
	GUIState->ColorTable[GUIColorExt_VioletRed4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b2252"), "VioletRed4");
	GUIState->ColorTable[GUIColorExt_wheat] = GUICreateColorSlot(GUIState, GUIColorHex("#f5deb3"), "wheat");
	GUIState->ColorTable[GUIColorExt_wheat1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffe7ba"), "wheat1");
	GUIState->ColorTable[GUIColorExt_wheat2] = GUICreateColorSlot(GUIState, GUIColorHex("#eed8ae"), "wheat2");
	GUIState->ColorTable[GUIColorExt_wheat3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdba96"), "wheat3");
	GUIState->ColorTable[GUIColorExt_wheat4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b7e66"), "wheat4");
	GUIState->ColorTable[GUIColorExt_white] = GUICreateColorSlot(GUIState, GUIColorHex("#ffffff"), "white");
	GUIState->ColorTable[GUIColorExt_WhiteSmoke] = GUICreateColorSlot(GUIState, GUIColorHex("#f5f5f5"), "WhiteSmoke");
	GUIState->ColorTable[GUIColorExt_yellow1] = GUICreateColorSlot(GUIState, GUIColorHex("#ffff00"), "yellow1");
	GUIState->ColorTable[GUIColorExt_yellow2] = GUICreateColorSlot(GUIState, GUIColorHex("#eeee00"), "yellow2");
	GUIState->ColorTable[GUIColorExt_yellow3] = GUICreateColorSlot(GUIState, GUIColorHex("#cdcd00"), "yellow3");
	GUIState->ColorTable[GUIColorExt_yellow4] = GUICreateColorSlot(GUIState, GUIColorHex("#8b8b00"), "yellow4");
	GUIState->ColorTable[GUIColorExt_YellowGreen] = GUICreateColorSlot(GUIState, GUIColorHex("#9acd32"), "YellowGreen");
}
#endif

void GUIInitState(gui_state* GUIState, font_info* FontInfo, input_system* Input, i32 Width, i32 Height) {
	GUIState->RenderStack = 0;
	GUIState->FontInfo = FontInfo;

	GUIState->Input = Input;

	GUIState->FontScale = 1.0f;

	GUIState->ScreenWidth = Width;
	GUIState->ScreenHeight = Height;

	GUIState->PlusMinusSymbol = 0;

	GUIState->GUIMem = AllocateStackedMemory(KILOBYTES(100));

#if 0
	GUIState->TempRect.Rect.Min = V2(400, 400);
	GUIState->TempRect.Rect.Max = V2(600, 600);
	GUIState->TempRect.SizeInteraction = {};
	GUIState->TempRect.PosInteraction = {};
#endif

	//NOTE(Dima): Initialization of the root node;
	GUIState->RootNode = PushStruct(&GUIState->GUIMem, gui_element);
	gui_element* RootNode = GUIState->RootNode;

	RootNode->PrevBro = RootNode;
	RootNode->NextBro = RootNode;

	CopyStrings(RootNode->Name, "MainRoot");
	CopyStrings(RootNode->Text, "MainRoot");

	RootNode->ID = GUIStringHashFNV(RootNode->Name);
	RootNode->Expanded = 1;
	RootNode->Depth = 0;

	RootNode->Parent = 0;
	RootNode->Type = GUIElement_None;

	RootNode->ChildrenSentinel = PushStruct(&GUIState->GUIMem, gui_element);

	RootNode->ChildrenSentinel->NextBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->PrevBro = RootNode->ChildrenSentinel;
	RootNode->ChildrenSentinel->Parent = RootNode;

	RootNode->NextBro = RootNode;
	RootNode->PrevBro = RootNode;

	RootNode->TempParent = 0;
	RootNode->TempParentTree = 0;

	GUIState->WalkaroundEnabled = false;
	GUIState->WalkaroundIsHot = false;
	GUIState->WalkaroundElement = RootNode;

	GUIState->CurrentNode = RootNode;
	GUIState->CurrentTreeParent = RootNode;

	/*
	NOTE(Dima):
	Initialization of the "Free store" of the gui elements;
	Used for freeing and allocating static gui elements;
	Static gui elements has no cache and can be freed every frame;
	*/
	GUIState->FreeElementsSentinel = PushStruct(&GUIState->GUIMem, gui_element);
	GUIState->FreeElementsSentinel->NextBro = GUIState->FreeElementsSentinel;
	GUIState->FreeElementsSentinel->PrevBro = GUIState->FreeElementsSentinel;

	//NOTE(Dima): Initialization of view sentinel
	GUIState->ViewSentinel = PushStruct(&GUIState->GUIMem, gui_view);
	GUIState->ViewSentinel->NextBro = GUIState->ViewSentinel;
	GUIState->ViewSentinel->PrevBro = GUIState->ViewSentinel;

	//NOTE(Dima): Initialization of view free list sentinel element
	GUIState->FreeViewSentinel = PushStruct(&GUIState->GUIMem, gui_view);
	GUIState->FreeViewSentinel->NextBro = GUIState->FreeViewSentinel;
	GUIState->FreeViewSentinel->PrevBro = GUIState->FreeViewSentinel;

	//NOTE(DIMA): Allocating and initializing default view
	gui_view* DefaultView = PushStruct(&GUIState->GUIMem, gui_view);
	*DefaultView = {};
	DefaultView->NextBro = GUIState->ViewSentinel->NextBro;
	DefaultView->PrevBro = GUIState->ViewSentinel;

	DefaultView->NextBro->PrevBro = DefaultView;
	DefaultView->PrevBro->NextBro = DefaultView;

	DefaultView->ID = GUIStringHashFNV("DefaultView");
	CopyStrings(DefaultView->Name, "DefaultView");

	DefaultView->ViewType = GUIView_Tree;

	DefaultView->CurrentX = 0;
	DefaultView->CurrentY = GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;

	GUIState->DefaultView = DefaultView;
	GUIState->CurrentView = DefaultView;

	//NOTE(Dima): Initialization of the color table
	GUIState->ColorTable[GUIColor_Black] = GUICreateColorSlot(GUIState, V4(0.0f, 0.0f, 0.0f, 1.0f), "Black");
	GUIState->ColorTable[GUIColor_White] = GUICreateColorSlot(GUIState, V4(1.0f, 1.0f, 1.0f, 1.0f), "White");
	GUIState->ColorTable[GUIColor_Red] = GUICreateColorSlot(GUIState, V4(1.0f, 0.0f, 0.0f, 1.0f), "Red");
	GUIState->ColorTable[GUIColor_Green] = GUICreateColorSlot(GUIState, V4(0.0f, 1.0f, 0.0f, 1.0f), "Green");
	GUIState->ColorTable[GUIColor_Blue] = GUICreateColorSlot(GUIState, V4(0.0f, 0.0f, 1.0f, 1.0f), "Blue");
	GUIState->ColorTable[GUIColor_Yellow] = GUICreateColorSlot(GUIState, V4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow");
	GUIState->ColorTable[GUIColor_Magenta] = GUICreateColorSlot(GUIState, V4(1.0f, 0.0f, 1.0f, 1.0f), "Magenta");
	GUIState->ColorTable[GUIColor_Cyan] = GUICreateColorSlot(GUIState, V4(0.0f, 1.0f, 1.0f, 1.0f), "Cyan");

	GUIState->ColorTable[GUIColor_PrettyBlue] = GUICreateColorSlot(GUIState, V4(0.0f, 0.5f, 1.0f, 1.0f), "PrettyBlue");
	GUIState->ColorTable[GUIColor_PrettyGreen] = GUICreateColorSlot(GUIState, V4(0.5f, 1.0f, 0.0f, 1.0f), "PrettyGreen");

	GUIState->ColorTable[GUIColor_Purple] = GUICreateColorSlot(GUIState, GUIColor255(85, 26, 139), "Purple");
	GUIState->ColorTable[GUIColor_DarkRed] = GUICreateColorSlot(GUIState, GUIColorHex("#cd0000"), "DarkRed");
	GUIState->ColorTable[GUIColor_Orange] = GUICreateColorSlot(GUIState, GUIColorHex("#ffa500"), "Orange");
	GUIState->ColorTable[GUIColor_OrangeRed] = GUICreateColorSlot(GUIState, GUIColorHex("#ff4500"), "OrangeRed");
	GUIState->ColorTable[GUIColor_BloodOrange] = GUICreateColorSlot(GUIState, GUIColorHex("#D1001C"), "BloodOrange");
	GUIState->ColorTable[GUIColor_Amber] = GUICreateColorSlot(GUIState, GUIColorHex("#FF7E00"), "Amber");
	GUIState->ColorTable[GUIColor_Brown] = GUICreateColorSlot(GUIState, GUIColorHex("#993300"), "Brown");
	GUIState->ColorTable[GUIColor_RoyalBlue] = GUICreateColorSlot(GUIState, GUIColorHex("#436eee"), "RoyalBlue");
	GUIState->ColorTable[GUIColor_PrettyPink] = GUICreateColorSlot(GUIState, GUIColorHex("#ee30a7"), "PrettyPink");
	GUIState->ColorTable[GUIColor_Cerise] = GUICreateColorSlot(GUIState, GUIColorHex("#DE3163"), "Cerise");
	GUIState->ColorTable[GUIColor_CerisePink] = GUICreateColorSlot(GUIState, GUIColorHex("#EC3B83"), "CerisePink");
	GUIState->ColorTable[GUIColor_ChinaPink] = GUICreateColorSlot(GUIState, GUIColorHex("#DE6FA1"), "ChinaPink");
	GUIState->ColorTable[GUIColor_ChinaRose] = GUICreateColorSlot(GUIState, GUIColorHex("#A8516E"), "ChinaRose");

	GUIState->ColorTable[GUIColor_BluishGray] = GUICreateColorSlot(GUIState, GUIColorHex("#778899"), "BluishGray");

	GUIState->ColorTable[GUIColor_Burlywood] = GUICreateColorSlot(GUIState, GUIColorHex("#deb887"), "Burlywood");
	GUIState->ColorTable[GUIColor_DarkGoldenrod] = GUICreateColorSlot(GUIState, GUIColorHex("#b8860b"), "DarkGoldenrod");
	GUIState->ColorTable[GUIColor_OliveDrab] = GUICreateColorSlot(GUIState, GUIColorHex("#6b8e23"), "OliveDrab");

	GUIState->ColorTable[GUIColor_Black_x20] = GUICreateColorSlot(GUIState, GUIColorHex("#202020"), "Black20");

#if GUI_EXT_COLORS_ENABLED
	GUIExtInitColors(GUIState);
#endif

	//NOTE(DIMA): Initialization of the color theme
	GUIState->ColorTheme = GUIDefaultColorTheme();
}

void GUIBeginTempRenderStack(gui_state* GUIState, render_stack* Stack) {
	GUIState->TempRenderStack = GUIState->RenderStack;
	GUIState->RenderStack = Stack;
}

void GUIEndTempRenderStack(gui_state* GUIState) {
	GUIState->RenderStack = GUIState->TempRenderStack;
}

void GUIBeginFrame(gui_state* GUIState, render_stack* RenderStack) {
	GUIState->RenderStack = RenderStack;
}

inline b32 GUIElementIsValidForWalkaround(gui_element* Element) {
	b32 Result = 0;

	if (Element) {
		gui_element* Parent = Element->Parent;

		if (Parent) {
			if (Parent->ChildrenSentinel != Element) {
				Result =
					(Element->Type != GUIElement_StaticItem) &&
					(Element->Type != GUIElement_Row) &&
					(Element->Type != GUIElement_None);
			}
		}
		else {
			//NOTE(DIMA): This is the root element
			Assert(Element->Type == GUIElement_None);
			Result = 0;
		}
	}

	return(Result);
}

inline b32 GUIElementIsSentinelOfRow(gui_element* Element) {
	b32 Result = 0;

	if (Element) {
		gui_element* Parent = Element->Parent;
		if (Element == Parent->ChildrenSentinel &&
			Parent->Type == GUIElement_Row)
		{
			Result = 1;
		}
	}

	return(Result);
}

enum gui_walkaround_type {
	GUIWalkaround_None,

	GUIWalkaround_Next,
	GUIWalkaround_Prev,
};

inline gui_element* GUIWalkaroundStep(gui_element* Elem, u32 WalkaroundType) {
	gui_element* Result = 0;

	if (WalkaroundType == GUIWalkaround_Next) {
		Result = Elem->NextBro;
	}
	else if (WalkaroundType == GUIWalkaround_Prev) {
		Result = Elem->PrevBro;
	}
	else {
		Assert(!"Invalid walkaround type");
	}

	return(Result);
}

inline gui_element* GUIFindNextForWalkaroundInRow(gui_element* Row, u32 WalkaroundType) {
	gui_element* Result = 0;

	gui_element* At = GUIWalkaroundStep(Row->ChildrenSentinel, WalkaroundType);

	while (At != Row->ChildrenSentinel) {
		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}
		else {
			if (At->Type == GUIElement_Row) {
				Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
				if (Result) {
					return(Result);
				}
			}
		}

		At = GUIWalkaroundStep(At, WalkaroundType);
	}

	return(Result);
}

inline gui_element* GUIFindForWalkaround(gui_element* Element, u32 WalkaroundType) {
	gui_element* Result = 0;

	gui_element* At = GUIWalkaroundStep(Element, WalkaroundType);
	while (At != Element) {

		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}
		else {
#if 1
			if (GUIElementIsSentinelOfRow(At)) {
				At = At->Parent;
				At = GUIWalkaroundStep(At, WalkaroundType);
				continue;
			}

			if (At->Type == GUIElement_Row) {
				Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
				if (Result) {
					break;
				}
			}
#endif
		}

		At = GUIWalkaroundStep(At, WalkaroundType);
	}

	return(Result);
}

static gui_element* GUIFindNextForWalkaround(gui_element* Element) {
	gui_element* Result = GUIFindForWalkaround(Element, GUIWalkaround_Next);

	return(Result);
}

static gui_element* GUIFindPrevForWalkaround(gui_element* Element) {
	gui_element* Result = GUIFindForWalkaround(Element, GUIWalkaround_Prev);

	return(Result);
}

inline gui_element* GUIFindElementForWalkaroundBFS(gui_element* CurrentElement) {
	gui_element* Result = 0;

	u32 WalkaroundType = GUIWalkaround_Prev;

	if (CurrentElement->ChildrenSentinel) {
		gui_element* At = GUIWalkaroundStep(CurrentElement->ChildrenSentinel, WalkaroundType);

		while (At != CurrentElement->ChildrenSentinel) {

			if (GUIElementIsValidForWalkaround(At)) {
				Result = At;
				return(Result);
			}
			else {
#if 1
				if (GUIElementIsSentinelOfRow(At)) {
					At = At->Parent;
					At = GUIWalkaroundStep(At, WalkaroundType);
					continue;
				}

				if (At->Type == GUIElement_Row) {
					//Result = GUIFindNextForWalkaroundInRow(At, WalkaroundType);
					Result = GUIFindElementForWalkaroundBFS(At);
					if (Result) {
						return(Result);
					}
				}
#endif
			}

			At = GUIWalkaroundStep(At, WalkaroundType);
		}

		At = GUIWalkaroundStep(CurrentElement->ChildrenSentinel, WalkaroundType);
		while (At != CurrentElement->ChildrenSentinel) {
			Result = GUIFindElementForWalkaroundBFS(At);
			if (Result) {
				return(Result);
			}

			At = GUIWalkaroundStep(At, WalkaroundType);
		}
	}

	return(Result);
}

inline gui_element* GUIFindTrueParent(gui_element* Elem) {
	gui_element* Result = 0;

	gui_element* At = Elem->Parent;
	while (At) {
		if (GUIElementIsValidForWalkaround(At)) {
			Result = At;
			break;
		}

		At = At->Parent;
	}

	if (!Result) {
		Result = Elem;
	}

	return(Result);
}

inline b32 GUIWalkaroundIsOnElement(gui_state* State, gui_element* Element) {
	b32 Result = 0;

	if (State->WalkaroundEnabled) {
		if (State->WalkaroundElement == Element) {
			Result = 1;
		}
	}

	return(Result);
}

inline b32 GUIWalkaroundIsHere(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	b32 Result = GUIWalkaroundIsOnElement(State, State->CurrentNode);

	return(Result);
}

void GUIEndFrame(gui_state* GUIState) {

	//NOTE(DIMA): Processing walkaround
	if (ButtonWentDown(GUIState->Input, KeyType_Backquote)) {
		GUIState->WalkaroundEnabled = !GUIState->WalkaroundEnabled;
	}

	if (GUIState->WalkaroundEnabled) {
		gui_element** Walk = &GUIState->WalkaroundElement;

		if (!GUIElementIsValidForWalkaround(*Walk)) {
			*Walk = GUIFindElementForWalkaroundBFS(*Walk);
		}

		if (!GUIState->WalkaroundIsHot) {

			if (ButtonWentDown(GUIState->Input, KeyType_Up)) {
				gui_element* PrevElement = GUIFindNextForWalkaround(*Walk);

				if (PrevElement) {
					*Walk = PrevElement;
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Down)) {
				gui_element* NextElement = GUIFindPrevForWalkaround(*Walk);

				if (NextElement) {
					*Walk = NextElement;
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Right)) {
				if ((*Walk)->Expanded) {
					gui_element* FirstChildren = GUIFindElementForWalkaroundBFS(*Walk);

					if (FirstChildren) {
						*Walk = FirstChildren;
					}
				}
			}

			if (ButtonWentDown(GUIState->Input, KeyType_Left)) {
				*Walk = GUIFindTrueParent(*Walk);
			}
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
			if ((*Walk)->Type == GUIElement_TreeNode) {
				(*Walk)->Expanded = !(*Walk)->Expanded;
				(*Walk)->Cache.TreeNode.ExitState = GUITreeNodeExit_None;
			}

#if 0
			if ((*Walk)->Type == GUIElement_InteractibleItem) {
				GUIState->WalkaroundIsHot = !GUIState->WalkaroundIsHot;
			}
#endif
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Backspace)) {
			*Walk = GUIFindTrueParent(*Walk);
			if ((*Walk)->Expanded) {
				(*Walk)->Expanded = 0;
			}

			GUIState->WalkaroundIsHot = 0;
		}

		if (ButtonWentDown(GUIState->Input, KeyType_Tab)) {
			if (!(*Walk)->Expanded) {
				(*Walk)->Expanded = true;
			}
			gui_element* FirstChildren = GUIFindElementForWalkaroundBFS(*Walk);
			if (FirstChildren) {
				*Walk = FirstChildren;
			}
		}
	}

	//NOTE(DIMA): Resetting default view;
	gui_view* DefView = GUIState->DefaultView;
	DefView->CurrentX = 0;
	DefView->CurrentY = GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale;
}

inline gui_view* GUIAllocateViewElement(gui_state* State) {
	gui_view* View = 0;

	if (State->FreeViewSentinel->NextBro != State->FreeViewSentinel) {
		View = State->FreeViewSentinel->NextBro;

		View->NextBro->PrevBro = View->PrevBro;
		View->PrevBro->NextBro = View->NextBro;
	}
	else {
		View = PushStruct(&State->GUIMem, gui_view);
	}

	*View = {};

	return(View);
}

inline void GUIInsertViewElement(gui_state* State, gui_view* ToInsert) {
	ToInsert->NextBro = State->ViewSentinel->NextBro;
	ToInsert->PrevBro = State->ViewSentinel;

	ToInsert->NextBro->PrevBro = ToInsert;
	ToInsert->PrevBro->NextBro = ToInsert;
}

inline void GUIFreeViewElement(gui_state* State, gui_view* ToFree) {
	ToFree->NextBro->PrevBro = ToFree->PrevBro;
	ToFree->PrevBro->NextBro = ToFree->NextBro;

	ToFree->NextBro = State->FreeViewSentinel->NextBro;
	ToFree->PrevBro = State->FreeViewSentinel;

	ToFree->NextBro->PrevBro = ToFree;
	ToFree->PrevBro->NextBro = ToFree;
}

void GUIBeginView(gui_state* GUIState, char* ViewName, u32 ViewType) {
	/*
	NOTE(DIMA):
	Here I make unique id for view and calculate
	it's hash. Then I try to find view in Global
	list with the same ID. It's perfomance better
	than comparing two strings. But there is the
	caveat. We need to make sure that the hash
	calculation function is crypto-strong enough
	so that the possibility to have 2 same id's
	for different strings is very small.
	*/

	char IdBuf[256];
	stbsp_snprintf(
		IdBuf, sizeof(IdBuf),
		"%s_TreeID_%u",
		ViewName,
		GUITreeElementID(GUIState->CurrentNode));
	u32 IdBufHash = GUIStringHashFNV(IdBuf);

	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_View, IdBuf, 0);
	gui_element* CurrentElement = GUIGetCurrentElement(GUIState);
	gui_view* ParentView = GUIState->CurrentView;

	if (!CurrentElement->Cache.IsInitialized) {
		//IMPORTANT(DIMA): Think about how to choose position that we want
		CurrentElement->Cache.View.Position = V2(ParentView->CurrentX, ParentView->CurrentY);
		CurrentElement->Cache.View.Dimension = V2(100, 100);

		CurrentElement->Cache.IsInitialized = true;
	}
	v2* ViewPosition = &CurrentElement->Cache.View.Position;


	//NOTE(Dima): Find view in the existing list
	gui_view* View = 0;
	for (gui_view* At = GUIState->ViewSentinel->NextBro;
		At != GUIState->ViewSentinel;
		At = At->NextBro)
	{
		if (IdBufHash == At->ID) {
			View = At;
			break;
		}
	}

	//NOTE(Dima): View not found. Should allocate it
	if (View == 0) {
		View = GUIAllocateViewElement(GUIState);
		GUIInsertViewElement(GUIState, View);

		View->ID = IdBufHash;
		View->ViewType = ViewType;
		View->Parent = ParentView;
	}

	View->CurrentX = ViewPosition->x;
	View->CurrentY = ViewPosition->y;

	GUIState->CurrentView = View;

	if (ViewType == GUIView_Tree) {
		GUITreeBegin(GUIState, ViewName);
	}
	else if (ViewType == GUIView_Window) {

	}
}

void GUIEndView(gui_state* GUIState, u32 ViewType) {

	gui_view* View = GUIGetCurrentView(GUIState);

	Assert(View->ViewType == ViewType);

	if (ViewType == GUIView_Tree) {
		GUITreeEnd(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_View);

	GUIState->CurrentView = GUIState->CurrentView->Parent;
}

void GUIBeginRow(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	Assert(!View->RowBeginned);

	char NameBuf[16];
	stbsp_sprintf(NameBuf, "Row:%u", State->CurrentNode->RowCount);

	GUIBeginElement(State, GUIElement_Row, NameBuf, 0);

	View->RowBeginX = View->CurrentX;
	View->RowBeginned = true;
	View->RowBiggestHeight = 0;
}

void GUIEndRow(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	Assert(View->RowBeginned);

	b32 NeedShow = GUIElementShouldBeUpdated(State->CurrentNode);

	if (NeedShow) {
		View->CurrentX = View->RowBeginX;
		View->CurrentY += View->RowBiggestHeight + GetNextRowAdvance(State->FontInfo) * 0.2f;
	}

	View->RowBeginned = false;
	View->RowBiggestHeight = 0;

	GUIEndElement(State, GUIElement_Row);

	State->CurrentNode->RowCount++;
}

inline void GUIPreAdvanceCursor(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	gui_element* Element = State->CurrentNode;

	View->CurrentPreAdvance = (Element->Depth - 1) * 2 * State->FontScale * State->FontInfo->AscenderHeight;
	View->CurrentX += View->CurrentPreAdvance;
}

inline void GUIDescribeElementDim(gui_state* State, v2 ElementDim) {
	gui_view* View = GUIGetCurrentView(State);

	View->LastElementWidth = ElementDim.x;
	View->LastElementHeight = ElementDim.y;

	if ((ElementDim.y > View->RowBiggestHeight) && View->RowBeginned) {
		View->RowBiggestHeight = ElementDim.y;
	}
}

inline void GUIAdvanceCursor(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	if (View->RowBeginned) {
		View->CurrentX += View->LastElementWidth + State->FontInfo->AscenderHeight * State->FontScale;
	}
	else {
#if 0
		View->CurrentY += GetNextRowAdvance(State->FontInfo, 1.2f);
#else
		View->CurrentY += View->LastElementHeight + GetNextRowAdvance(State->FontInfo) * 0.2f;
#endif
	}

	View->CurrentX -= View->CurrentPreAdvance;
}

inline gui_element* GUIAllocateListElement(gui_state* State) {
	gui_element* Element = 0;

	if (State->FreeElementsSentinel->NextBro != State->FreeElementsSentinel) {
		Element = State->FreeElementsSentinel->NextBro;

		Element->NextBro->PrevBro = Element->PrevBro;
		Element->PrevBro->NextBro = Element->NextBro;
	}
	else {
		Element = PushStruct(&State->GUIMem, gui_element);
	}

	return(Element);
}

inline void GUIInsertListElement(gui_element* Sentinel, gui_element* ToInsert) {
	ToInsert->NextBro = Sentinel->NextBro;
	ToInsert->PrevBro = Sentinel;

	ToInsert->PrevBro->NextBro = ToInsert;
	ToInsert->NextBro->PrevBro = ToInsert;
}

inline void GUIFreeListElement(gui_state* State, gui_element* Element) {
	Element->NextBro->PrevBro = Element->PrevBro;
	Element->PrevBro->NextBro = Element->NextBro;

	Element->NextBro = State->FreeElementsSentinel->NextBro;
	Element->PrevBro = State->FreeElementsSentinel;

	Element->NextBro->PrevBro = Element;
	Element->PrevBro->NextBro = Element;
}

inline void GUIInitElementChildrenSentinel(gui_state* GUIState, gui_element* Element) {
	Element->ChildrenSentinel = GUIAllocateListElement(GUIState);
	Element->ChildrenSentinel->NextBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->PrevBro = Element->ChildrenSentinel;
	Element->ChildrenSentinel->Parent = Element;
}

static gui_element* GUIRequestElement(
	gui_state* GUIState,
	u32 ElementType,
	char* ElementName,
	gui_interaction* Interaction)
{
	gui_element* Parent = GUIState->CurrentNode;

	gui_element* Element = 0;

	b32 ElementIsDynamic =
		(ElementType != GUIElement_None &&
			ElementType != GUIElement_StaticItem);

	u32 ElementHash = 0;
	if (ElementIsDynamic)
	{
		ElementHash = GUIStringHashFNV(ElementName);

		//NOTE(DIMA): Finding the element in the hierarchy
		for (gui_element* Node = Parent->ChildrenSentinel->NextBro;
			Node != Parent->ChildrenSentinel;
			Node = Node->NextBro)
		{
			//TODO(Dima): Test perfomance
#if 1
			if (StringsAreEqual(ElementName, Node->Name)) {
#else
			if (ElementHash == Node->ID) {
#endif
				Element = Node;
				break;
			}
			}
		}

	if (ElementType == GUIElement_StaticItem) {
		Element = GUIAllocateListElement(GUIState);
		GUIInsertListElement(Parent->ChildrenSentinel, Element);

		Element->Expanded = 1;
		Element->Depth = Parent->Depth + 1;
		Element->RowCount = 0;
		Element->ID = 0;

		Element->ChildrenSentinel = 0;
	}

	//NOTE(Dima): Element not exist or not found. We should allocate it
	if (Element == 0) {
		//NOTE(DIMA): If the "Free store" of elements is not empty, get the memory from there
		//TODO(DIMA): Some elements memory might be initialzed again if we get it from here
		Element = GUIAllocateListElement(GUIState);
		GUIInsertListElement(Parent->ChildrenSentinel, Element);

		//NOTE(Dima): Pre-Setting common values
		Element->TempParentTree = GUIState->CurrentTreeParent;

		if ((ElementType == GUIElement_TreeNode) ||
			(ElementType == GUIElement_CachedItem))
		{
			Element->Expanded = 1;
			Element->Depth = Parent->Depth + 1;

			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_Row) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth;

			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		if (ElementType == GUIElement_InteractibleItem) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth + 1;

			Element->ChildrenSentinel = 0;
		}

		if (ElementType == GUIElement_View) {
			Element->Expanded = 1;
			Element->Depth = Parent->Depth;

			GUIInitElementChildrenSentinel(GUIState, Element);
		}

		CopyStrings(Element->Name, ElementName);
		CopyStrings(Element->Text, ElementName);

		Element->ID = ElementHash;
		Element->RowCount = 0;
		Element->Cache = {};
	}

	//NOTE(Dima): Post-Setting common values
	Element->Type = ElementType;
	Element->Parent = Parent;
	Element->TempParent = 0;
	Element->RowCount = 0;

	//NOTE(Dima): Setting interaction ID for dynamic(cached) elements
	if (ElementIsDynamic && Interaction) {
		Interaction->ID = GUITreeElementID(Element);
	}

	return(Element);
	}

b32 GUIBeginElement(
	gui_state* State,
	u32 ElementType,
	char* ElementName,
	gui_interaction* ElementInteraction)
{
	gui_element* Element = GUIRequestElement(State, ElementType, ElementName, ElementInteraction);

	State->CurrentNode = Element;

	if (Element->Type == GUIElement_TreeNode) {
		State->CurrentTreeParent = Element;
	}

	b32 NeedShow = GUIElementShouldBeUpdated(State->CurrentNode);

	return(NeedShow);
}

void GUIEndElement(gui_state* State, u32 ElementType) {
	gui_element* Element = State->CurrentNode;

	Assert(ElementType == Element->Type);

#if 1
	gui_view* View = GUIGetCurrentView(State);

	if (GUIElementShouldBeUpdated(Element)) {
		//NOTE(Dima): Here I remember view Y for exit sliding effect
		gui_element* CurrentTreeParent = State->CurrentTreeParent;
		Assert((CurrentTreeParent->Type == GUIElement_TreeNode) ||
			(CurrentTreeParent == State->RootNode));
		CurrentTreeParent->Cache.TreeNode.StackY = View->CurrentY;
	}
#endif

	if (ElementType == GUIElement_StaticItem) {
		GUIFreeListElement(State, Element);
	}

	if (ElementType == GUIElement_Row) {
		//View->CurrentNode->Parent->RowCount = 0;
	}

	if (ElementType == GUIElement_TreeNode) {
		State->CurrentTreeParent = Element->TempParentTree;
	}


	State->CurrentNode = Element->Parent;
}

enum print_text_type {
	PrintTextType_PrintText,
	PrintTextType_GetTextSize,
};

static rect2 PrintTextInternal(gui_state* State, u32 Type, char* Text, float Px, float Py, float Scale, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {

	rect2 TextRect = {};

	float CurrentX = Px;
	float CurrentY = Py;

	char* At = Text;

	font_info* FontInfo = State->FontInfo;
	render_stack* Stack = State->RenderStack;

	while (*At) {
		int GlyphIndex = FontInfo->CodepointToGlyphMapping[*At];
		glyph_info* Glyph = &FontInfo->Glyphs[GlyphIndex];

		float BitmapScale = Glyph->Height * Scale;

		if (Type == PrintTextType_PrintText) {
			float BitmapMinX;
			float BitmapMinY;

			BitmapMinY = CurrentY + (Glyph->YOffset - 1.0f) * Scale;
			BitmapMinX = CurrentX + (Glyph->XOffset - 1.0f) * Scale;

			PushBitmap(Stack, &Glyph->Bitmap, { BitmapMinX + 2, BitmapMinY + 2 }, BitmapScale, V4(0.0f, 0.0f, 0.0f, 1.0f));
			PushBitmap(Stack, &Glyph->Bitmap, { BitmapMinX, BitmapMinY }, BitmapScale, Color);
		}

		float Kerning = 0.0f;
		if (*(At + 1)) {
			Kerning = GetKerningForCharPair(FontInfo, *At, *(At + 1));
		}

		CurrentX += (Glyph->Advance + Kerning) * Scale;

		++At;
	}

	TextRect.Min.x = Px;
	TextRect.Min.y = Py - FontInfo->AscenderHeight * Scale;
	TextRect.Max.x = CurrentX;
	TextRect.Max.y = Py - FontInfo->DescenderHeight * Scale;

	return(TextRect);
}

void GUILabel(gui_state* GUIState, char* LabelText, v2 At) {
	PrintTextInternal(GUIState, PrintTextType_PrintText, LabelText, At.x, At.y, 1.0f, GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));
}

void GUIAnchor(gui_state* GUIState, char* Name, v2 Pos, v2 Dim, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, Interaction);

	if (NeedShow) {
		rect2 WorkRect = Rect2MinDim(Pos, Dim);
		v4 WorkColor = GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor);
		v2 MouseP = GUIState->Input->MouseP;

		gui_element* Anchor = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &Anchor->Cache;

		if (!Cache->IsInitialized) {
			Cache->Anchor.OffsetInAnchor = {};

			Cache->IsInitialized = true;
		}
		v2* OffsetInAnchor = &Cache->Anchor.OffsetInAnchor;


		b32 IsHot = GUIInteractionIsHot(GUIState, Interaction);
		if (MouseInRect(GUIState->Input, WorkRect)) {
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				IsHot = GUISetInteractionHot(GUIState, Interaction, true);
				*OffsetInAnchor = MouseP - Pos;
			}
		}

		if (MouseButtonWentUp(GUIState->Input, MouseButton_Left)) {
			IsHot = GUISetInteractionHot(GUIState, Interaction, false);
			*OffsetInAnchor = {};
		}

		if (IsHot) {

			v2 WorkRectP = Interaction->ResizeContext.Position;
			/*Getting true position*/
			MouseP = MouseP - *OffsetInAnchor;

			float MinDimLen = GUIState->FontInfo->AscenderHeight * GUIState->FontScale;

			Assert((Interaction->Type == GUIInteraction_MoveInteraction) ||
				(Interaction->Type == GUIInteraction_ResizeInteraction));

			if (Interaction->Type == GUIInteraction_ResizeInteraction) {

				v2* WorkDim = Interaction->ResizeContext.DimensionPtr;
				switch (Interaction->ResizeContext.Type) {
					case GUIResizeInteraction_Default: {
						*WorkDim = MouseP - WorkRectP;

						if (MouseP.x - WorkRectP.x < MinDimLen) {
							WorkDim->x = MinDimLen;
						}

						if (MouseP.y - WorkRectP.y < MinDimLen) {
							WorkDim->y = MinDimLen;
						}
					}break;

					case GUIResizeInteraction_Horizontal: {
						if (MouseP.x - WorkRectP.x < MinDimLen) {
							WorkDim->x = MinDimLen;
						}
					}break;

					case GUIResizeInteraction_Vertical: {
						if (MouseP.y - WorkRectP.y < MinDimLen) {
							WorkDim->y = MinDimLen;
						}
					}break;

					case GUIResizeInteraction_Proportional: {
						float WidthToHeight = WorkDim->x / WorkDim->y;
						WorkDim->y = MouseP.y - WorkRectP.y;
						WorkDim->x = WorkDim->y * WidthToHeight;

						if (WorkDim->y < MinDimLen) {
							WorkDim->y = MinDimLen;
							WorkDim->x = WorkDim->y * WidthToHeight;
						}
					}break;
				}
			}
			else if (Interaction->Type == GUIInteraction_MoveInteraction) {
				v2* WorkP = Interaction->MoveContext.MovePosition;
				switch (Interaction->MoveContext.Type) {
					case GUIMoveInteraction_Move: {
						*WorkP = MouseP;
					}break;
				}
			}
		}


		PushRect(GUIState->RenderStack, WorkRect, WorkColor);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIWindow(gui_state* GUIState, char* Name, u32 CreationFlags, u32 Width, u32 Height) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0);

	if (NeedShow) {
		GUIPreAdvanceCursor(GUIState);

		gui_view* View = GUIGetCurrentView(GUIState);

		gui_element* Window = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &Window->Cache;

		if (!Cache->IsInitialized) {
			if (CreationFlags & GUIWindow_DefaultSize) {
				Cache->View.Dimension.x = 100;
				Cache->View.Dimension.y = 100;
			}
			else {
				Cache->View.Dimension.x = Width;
				Cache->View.Dimension.y = Height;
			}

			Cache->View.Position = V2(View->CurrentX, View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);

			Cache->IsInitialized = true;
		}

		v2* WindowDimension = &Cache->View.Dimension;
		v2* WindowPosition = &Cache->View.Position;

		int WindowOutlineWidth = 3;
		int InnerSubWindowWidth = 2;
		rect2 WindowRect = Rect2MinDim(*WindowPosition, *WindowDimension);

		PushRect(GUIState->RenderStack, WindowRect, GUIGetColor(GUIState, GUIState->ColorTheme.WindowBackgroundColor));
		PushRectOutline(GUIState->RenderStack, WindowRect, WindowOutlineWidth, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		if (CreationFlags & GUIWindow_TopBar) {
			float TopBarHeight = GUIState->FontScale * GetNextRowAdvance(GUIState->FontInfo);

			if (WindowDimension->y > TopBarHeight) {
				PushRect(
					GUIState->RenderStack,
					Rect2MinDim(
						*WindowPosition + V2(0, TopBarHeight),
						V2(WindowDimension->x, InnerSubWindowWidth)),
					GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));
			}
		}

		if (CreationFlags & GUIWindow_Resizable) {
			gui_interaction ResizeInteraction = GUIResizeInteraction(WindowRect.Min, WindowDimension, GUIResizeInteraction_Default);
			GUIAnchor(GUIState, "AnchorBR", WindowRect.Max, V2(5, 5), &ResizeInteraction);
		}

		gui_interaction MoveInteraction = GUIMoveInteraction(WindowPosition, GUIMoveInteraction_Move);
		GUIAnchor(GUIState, "AnchorMV", WindowRect.Min, V2(5, 5), &MoveInteraction);

		GUIDescribeElementDim(GUIState, *WindowDimension);
		GUIAdvanceCursor(GUIState);
	}
	GUIEndElement(GUIState, GUIElement_CachedItem);
}

void GUIImageView(gui_state* GUIState, char* Name, gui_interaction* Interaction) {
	GUITreeBegin(GUIState, Name);

	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, 0);

	if (NeedShow) {
		rgba_buffer* Buffer = Interaction->VariableLink.Value_RGBABuffer;

		gui_element* ImageView = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &ImageView->Cache;

		if (!Cache->IsInitialized) {
			/*
			Cache->ImageView.Dimension =
			V2(Interaction->VariableLink.Value_RGBABuffer->Width,
			Interaction->VariableLink.Value_RGBABuffer->Height);
			*/

			Cache->ImageView.Dimension = V2(
				(float)Interaction->VariableLink.Value_RGBABuffer->Width /
				(float)Interaction->VariableLink.Value_RGBABuffer->Height * 100, 100);
			Cache->IsInitialized = true;
		}
		v2* WorkDim = &Cache->ImageView.Dimension;

		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 ImageRect;
		ImageRect.Min = V2(View->CurrentX, View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);
		ImageRect.Max = ImageRect.Min + *WorkDim;

		PushBitmap(GUIState->RenderStack, Buffer, ImageRect.Min, GetRectHeight(ImageRect));
		PushRectOutline(GUIState->RenderStack, ImageRect, 3, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		gui_interaction ResizeInteraction = GUIResizeInteraction(ImageRect.Min, WorkDim, GUIResizeInteraction_Proportional);
		GUIAnchor(GUIState, "Anchor0", ImageRect.Max, V2(5, 5), &ResizeInteraction);

		GUIDescribeElementDim(GUIState, GetRectDim(ImageRect));
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);

	GUITreeEnd(GUIState);
}

void GUIStackedMemGraph(gui_state* GUIState, char* Name, gui_interaction* Interaction) {
	GUITreeBegin(GUIState, Name);

	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_CachedItem, Name, Interaction);

	Assert(Interaction->VariableLink.Type == GUIVarType_StackedMemory);

	if (NeedShow) {
		stacked_memory* WorkMem = Interaction->VariableLink.Value_StackedMemory;

		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		gui_element* StackedMem = GUIGetCurrentElement(GUIState);
		gui_element_cache* Cache = &StackedMem->Cache;

		if (!Cache->IsInitialized) {

			Cache->StackedMem.Dimension =
				V2(GUIState->FontInfo->AscenderHeight * 40.0f,
					GUIState->FontInfo->AscenderHeight * 3.0f);

			Cache->IsInitialized = true;
		}

		v2* WorkDim = &Cache->StackedMem.Dimension;

		rect2 GraphRect;
		GraphRect.Min.x = View->CurrentX;
		GraphRect.Min.y = View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		GraphRect.Max = GraphRect.Min + *WorkDim;

		v2 GraphRectDim = GetRectDim(GraphRect);

		u64 OccupiedCount = WorkMem->Used;
		u64 FreeCount = (u64)WorkMem->MaxSize - OccupiedCount;
		u64 TotalCount = WorkMem->MaxSize;

		float OccupiedPercentage = (float)OccupiedCount / (float)TotalCount;

		rect2 OccupiedRect = Rect2MinDim(GraphRect.Min, V2(GraphRectDim.x * OccupiedPercentage, GraphRectDim.y));
		rect2 FreeRect = Rect2MinDim(
			V2(OccupiedRect.Max.x, OccupiedRect.Min.y),
			V2(GraphRectDim.x * (1.0f - OccupiedPercentage), GraphRectDim.y));

		float Inner = 2.0f;
		float Outer = 3.0f;

		PushRect(GUIState->RenderStack, OccupiedRect, GUIGetColor(GUIState, GUIState->ColorTheme.FirstColor));
		PushRectOutline(GUIState->RenderStack, OccupiedRect, Inner, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));
		PushRect(GUIState->RenderStack, FreeRect, GUIGetColor(GUIState, GUIState->ColorTheme.SecondaryColor));
		PushRectOutline(GUIState->RenderStack, FreeRect, Inner, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		PushRectOutline(GUIState->RenderStack, GraphRect, Outer, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		if (MouseInRect(GUIState->Input, GraphRect)) {
			char InfoStr[128];
			stbsp_sprintf(
				InfoStr,
				"Occupied: %llu(%.2f%%); Total: %llu",
				OccupiedCount,
				(float)OccupiedCount / (float)TotalCount * 100.0f,
				TotalCount);

			GUILabel(GUIState, InfoStr, GUIState->Input->MouseP);
		}

		gui_interaction ResizeInteraction = GUIResizeInteraction(GraphRect.Min, WorkDim, GUIResizeInteraction_Default);
		GUIAnchor(GUIState, "Anchor0", GraphRect.Max, V2(5, 5), &ResizeInteraction);

		GUIDescribeElementDim(
			GUIState,
			V2(GraphRectDim.x + Outer,
				GraphRectDim.y + (2.0f * Outer)));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_CachedItem);
	GUITreeEnd(GUIState);
}

void GUIText(gui_state* GUIState, char* Text) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, Text, 0);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(
			GUIState,
			PrintTextType_PrintText,
			Text,
			View->CurrentX,
			View->CurrentY,
			GUIState->FontScale, GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(GUIState, GetRectDim(Rc));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

static void GUIValueView(gui_state* GUIState, gui_variable_link* Link, char* Name, float ViewMultiplier) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, "", 0);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		char ValueBuf[32];

		if (Link->Type == GUIVarType_F32) {
			stbsp_sprintf(ValueBuf, "%.2f", *Link->Value_F32);
		}
		else if (Link->Type == GUIVarType_B32) {
			if (*Link->Value_B32) {
				stbsp_sprintf(ValueBuf, "%s", "true");
			}
			else {
				stbsp_sprintf(ValueBuf, "%s", "false");
			}
		}
		else if (Link->Type == GUIVarType_U32) {
			stbsp_sprintf(ValueBuf, "%u", *Link->Value_U32);
		}
		else if (Link->Type == GUIVarType_I32) {
			stbsp_sprintf(ValueBuf, "%d", *Link->Value_I32);
		}

		char Buf[64];
		if (Name) {
			stbsp_sprintf(Buf, "%s: %s", Name, ValueBuf);
		}
		else {
			stbsp_sprintf(Buf, "%s", ValueBuf);
		}

		rect2 TxtSizeRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Buf, 0, 0, GUIState->FontScale);
		v2 TxtDim = GetRectDim(TxtSizeRc);

		rect2 WorkRect = Rect2MinDim(
			V2(View->CurrentX, View->CurrentY - GUIState->FontScale * GUIState->FontInfo->AscenderHeight),
			V2(GUIState->FontInfo->AscenderHeight* GUIState->FontScale * ViewMultiplier, TxtDim.y));

		v2 WorkRectDim = GetRectDim(WorkRect);

		PushRect(GUIState->RenderStack, WorkRect, GUIGetColor(GUIState, GUIState->ColorTheme.SecondaryColor));
		PushRectOutline(GUIState->RenderStack, WorkRect, 1, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		PrintTextInternal(
			GUIState,
			PrintTextType_PrintText,
			Buf,
			WorkRect.Min.x + WorkRectDim.x * 0.5f - TxtDim.x * 0.5f,
			View->CurrentY,
			GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));

		GUIDescribeElementDim(GUIState, WorkRectDim);
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

void GUIColorRectView(gui_state* GUIState, v4 Color) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_StaticItem, "", 0);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 WorkRect = Rect2MinDim(
			V2(View->CurrentX, View->CurrentY - GUIState->FontScale * GUIState->FontInfo->AscenderHeight),
			V2(GUIState->FontInfo->AscenderHeight * GUI_VALUE_COLOR_VIEW_MULTIPLIER, GetNextRowAdvance(GUIState->FontInfo) * GUIState->FontScale));

		PushRect(GUIState->RenderStack, WorkRect, Color);
		PushRectOutline(
			GUIState->RenderStack, WorkRect, 1,
			GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		GUIDescribeElementDim(GUIState, GetRectDim(WorkRect));
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_StaticItem);
}

void GUIColorView(gui_state* GUIState, v4 Color, char* Name) {
	GUIBeginRow(GUIState);

	if (Name) {
		GUIText(GUIState, Name);
	}

	int ValR = (int)(Color.r * 255.0f);
	int ValG = (int)(Color.g * 255.0f);
	int ValB = (int)(Color.b * 255.0f);
	int ValA = (int)(Color.a * 255.0f);

	gui_variable_link Link1 = GUIVariableLink(&ValR, GUIVarType_I32);
	gui_variable_link Link2 = GUIVariableLink(&ValG, GUIVarType_I32);
	gui_variable_link Link3 = GUIVariableLink(&ValB, GUIVarType_I32);
	gui_variable_link Link4 = GUIVariableLink(&ValA, GUIVarType_I32);

	GUIValueView(GUIState, &Link1, "R", GUI_VALUE_COLOR_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link2, "G", GUI_VALUE_COLOR_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link3, "B", GUI_VALUE_COLOR_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link4, "A", GUI_VALUE_COLOR_VIEW_MULTIPLIER);

	GUIColorRectView(GUIState, Color);

	GUIEndRow(GUIState);
}

void GUIVector2View(gui_state* GUIState, v2 Value, char* Name) {
	GUIBeginRow(GUIState);

	if (Name) {
		GUIText(GUIState, Name);
	}

	gui_variable_link Link1 = GUIVariableLink(&Value.x, GUIVarType_F32);
	gui_variable_link Link2 = GUIVariableLink(&Value.y, GUIVarType_F32);

	GUIValueView(GUIState, &Link1, 0, GUI_VALUE_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link2, 0, GUI_VALUE_VIEW_MULTIPLIER);

	GUIEndRow(GUIState);
}

void GUIVector3View(gui_state* GUIState, v3 Value, char* Name) {
	GUIBeginRow(GUIState);

	if (Name) {
		GUIText(GUIState, Name);
	}

	gui_variable_link Link1 = GUIVariableLink(&Value.x, GUIVarType_F32);
	gui_variable_link Link2 = GUIVariableLink(&Value.y, GUIVarType_F32);
	gui_variable_link Link3 = GUIVariableLink(&Value.z, GUIVarType_F32);

	GUIValueView(GUIState, &Link1, 0, GUI_VALUE_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link2, 0, GUI_VALUE_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link3, 0, GUI_VALUE_VIEW_MULTIPLIER);

	GUIEndRow(GUIState);
}

void GUIVector4View(gui_state* GUIState, v4 Value, char* Name) {
	GUIBeginRow(GUIState);

	if (Name) {
		GUIText(GUIState, Name);
	}

	gui_variable_link Link1 = GUIVariableLink(&Value.r, GUIVarType_F32);
	gui_variable_link Link2 = GUIVariableLink(&Value.g, GUIVarType_F32);
	gui_variable_link Link3 = GUIVariableLink(&Value.b, GUIVarType_F32);
	gui_variable_link Link4 = GUIVariableLink(&Value.a, GUIVarType_F32);

	GUIValueView(GUIState, &Link1, 0, GUI_VALUE_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link2, 0, GUI_VALUE_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link3, 0, GUI_VALUE_VIEW_MULTIPLIER);
	GUIValueView(GUIState, &Link4, 0, GUI_VALUE_VIEW_MULTIPLIER);

	GUIEndRow(GUIState);
}

inline void GUIActionTextAction(gui_interaction* Interaction) {
	if (Interaction) {
		if (Interaction->Type == GUIInteraction_VariableLink) {
			if (Interaction->VariableLink.Type = GUIVarType_B32)
			{
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}
	}
}

void GUIActionText(gui_state* GUIState, char* Text, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Text, 0);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 Rc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, Text, View->CurrentX, View->CurrentY, GUIState->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		v4 TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);
		if (MouseInRect(GUIState->Input, Rc)) {
			TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				GUIActionTextAction(Interaction);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			PushRectOutline(GUIState->RenderStack, Rc, 2, GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor));

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUIActionTextAction(Interaction);
			}
		}

		PrintTextInternal(GUIState, PrintTextType_PrintText, Text, View->CurrentX, View->CurrentY, GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(GUIState, GetRectDim(Rc));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIButton(gui_state* GUIState, char* ButtonName, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, ButtonName, 0);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 NameRc = PrintTextInternal(
			GUIState,
			PrintTextType_PrintText,
			ButtonName,
			View->CurrentX,
			View->CurrentY,
			GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));

		v2 NameDim = V2(NameRc.Max.x - NameRc.Min.x, NameRc.Max.y - NameRc.Min.y);

		float OutlineWidth = 1;
		v4 ButRectHighlight = GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor);
		v4 TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);

		if (MouseInRect(GUIState->Input, NameRc)) {
			TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			PushRectOutline(GUIState->RenderStack, NameRc, 2, GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor));

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		PushRect(GUIState->RenderStack, NameRc, GUIGetColor(GUIState, GUIState->ColorTheme.FirstColor));
		PushRectOutline(GUIState->RenderStack, NameRc, OutlineWidth, ButRectHighlight);

		PrintTextInternal(GUIState, PrintTextType_PrintText, ButtonName, View->CurrentX, View->CurrentY, GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(GUIState, V2(NameRc.Max.x - View->CurrentX, NameRc.Max.y - NameRc.Min.y + OutlineWidth));
		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIBoolButton(gui_state* GUIState, char* ButtonName, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, ButtonName, 0);

	if (NeedShow) {

		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		rect2 NameRc = PrintTextInternal(
			GUIState,
			PrintTextType_PrintText,
			ButtonName,
			View->CurrentX,
			View->CurrentY,
			GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));
		v2 NameDim = V2(NameRc.Max.x - NameRc.Min.x, NameRc.Max.y - NameRc.Min.y);

		float PrintButX = View->CurrentX + NameDim.x + GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float PrintButY = View->CurrentY;

		rect2 FalseRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "false", PrintButX, PrintButY, GUIState->FontScale);
		v2 FalseDim = GetRectDim(FalseRc);
		rect2 ButRc = Rect2MinDim(V2(PrintButX, PrintButY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale), FalseDim);

		float OutlineWidth = 1;

		char TextToPrint[8];
		if (*Interaction->VariableLink.Value_B32) {
			rect2 TrueRc = PrintTextInternal(GUIState, PrintTextType_GetTextSize, "true", PrintButX, PrintButY, GUIState->FontScale);
			v2 TrueDim = GetRectDim(TrueRc);
			PrintButX = FalseRc.Min.x + (FalseDim.x - TrueDim.x) * 0.5f;

			stbsp_sprintf(TextToPrint, "%s", "true");
		}
		else {
			stbsp_sprintf(TextToPrint, "%s", "false");
		}

		v4 ButRectHighlight = GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor);
		v4 TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextColor);
		if (MouseInRect(GUIState->Input, ButRc)) {
			TextHighlightColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);
			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			PushRectOutline(GUIState->RenderStack, NameRc, 2, GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor));

			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				*Interaction->VariableLink.Value_B32 = !(*Interaction->VariableLink.Value_B32);
			}
		}

		PushRect(GUIState->RenderStack, ButRc, GUIGetColor(GUIState, GUIState->ColorTheme.FirstColor));
		PushRectOutline(GUIState->RenderStack, ButRc, OutlineWidth, ButRectHighlight);

		PrintTextInternal(GUIState, PrintTextType_PrintText, TextToPrint, PrintButX, PrintButY, GUIState->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(GUIState, V2(FalseRc.Max.x - View->CurrentX, FalseRc.Max.y - FalseRc.Min.y + OutlineWidth));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUIVerticalSlider(gui_state* State, char* Name, float Min, float Max, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(State, GUIElement_InteractibleItem, Name, Interaction);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(State);
		gui_vertical_slider_cache* Cache = &State->CurrentNode->Cache.VerticalSlider;
		float FontScale = State->FontScale;
		float SmallTextScale = FontScale * 0.8f;
		float NextRowAdvanceFull = GetNextRowAdvance(State->FontInfo, FontScale);
		float NextRowAdvanceSmall = GetNextRowAdvance(State->FontInfo, SmallTextScale);

		v2 WorkRectDim = V2(NextRowAdvanceFull, NextRowAdvanceFull * 5);

		GUIPreAdvanceCursor(State);

		float* Value = Interaction->VariableLink.Value_F32;
		Assert(Max > Min);

		//NOTE(DIMA): Calculating Max text rectangle
		//TODO(DIMA): Cache theese calculations
		char MaxValueTxt[16];
		stbsp_sprintf(MaxValueTxt, "%.2f", Max);
		v2 MaxValueRcMin = V2(View->CurrentX, View->CurrentY);
		rect2 MaxValueRcSize = PrintTextInternal(
			State,
			PrintTextType_GetTextSize,
			MaxValueTxt,
			0, 0,
			SmallTextScale);
		v2 MaxValueRcDim = GetRectDim(MaxValueRcSize);

		//NOTE(DIMA): Calculating vertical rectangle
		v2 WorkRectMin = V2(View->CurrentX, MaxValueRcMin.y - State->FontInfo->DescenderHeight * SmallTextScale);
		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);

		//NOTE(DIMA): Drawing vertical rectangle
		i32 RectOutlineWidth = 1;

		PushRect(State->RenderStack, WorkRect, GUIGetColor(State, State->ColorTheme.FirstColor));
		PushRectOutline(State->RenderStack, WorkRect, RectOutlineWidth, GUIGetColor(State, State->ColorTheme.OutlineColor));

		//NOTE(DIMA): Calculating Min text rectangle
		//TODO(DIMA): Cache theese calculations
		char MinValueTxt[16];
		stbsp_sprintf(MinValueTxt, "%.2f", Min);
		v2 MinValueRcMin = V2(WorkRect.Min.x, WorkRect.Max.y + NextRowAdvanceSmall);
		rect2 MinValueRcSize = PrintTextInternal(
			State,
			PrintTextType_GetTextSize,
			MinValueTxt,
			0, 0,
			SmallTextScale);
		v2 MinValueRcDim = GetRectDim(MaxValueRcSize);

		//NOTE(DIMA): Drawing Max value text
		rect2 MaxValueRc = PrintTextInternal(
			State,
			PrintTextType_PrintText,
			MaxValueTxt,
			MaxValueRcMin.x + 0.5f * (WorkRectDim.x - MaxValueRcDim.x),
			MaxValueRcMin.y,
			SmallTextScale,
			GUIGetColor(State, State->ColorTheme.TextColor));

		//NOTE(DIMA): Printing Min value text
		rect2 MinValueRc = PrintTextInternal(
			State,
			PrintTextType_PrintText,
			MinValueTxt,
			MinValueRcMin.x + 0.5f * (WorkRectDim.x - MinValueRcDim.x),
			MinValueRcMin.y,
			SmallTextScale,
			GUIGetColor(State, State->ColorTheme.TextColor));

		//NOTE(DIMA): Printing name of the element that consist from first 3 chars of the name
		char SmallTextToPrint[8];
		stbsp_sprintf(SmallTextToPrint, "%.4s", Name);
		char *SmallBufAt = SmallTextToPrint;
		while (*SmallBufAt) {
			char CurChar = *SmallBufAt;

			if (CurChar >= 'a' && CurChar <= 'z') {
				*SmallBufAt = CurChar - 'a' + 'A';
			}

			SmallBufAt++;
		}
		rect2 SmallTextRect = PrintTextInternal(State, PrintTextType_GetTextSize, SmallTextToPrint, 0, 0, SmallTextScale);
		v2 SmallTextRcDim = GetRectDim(SmallTextRect);

		float SmallTextX = WorkRect.Min.x + WorkRectDim.x * 0.5f - SmallTextRcDim.x * 0.5f;
		float SmallTextY = MinValueRc.Min.y + MinValueRcDim.y + State->FontInfo->AscenderHeight * SmallTextScale;

		rect2 SmallTextRc = PrintTextInternal(
			State,
			PrintTextType_PrintText,
			SmallTextToPrint,
			SmallTextX,
			SmallTextY,
			SmallTextScale,
			GUIGetColor(State, State->ColorTheme.TextColor));

		//NOTE(Dima): Processing the value
		float Range = Max - Min;
		if (*Value > Max) {
			*Value = Max;
		}
		else if (*Value < Min) {
			*Value = Min;
		}

		float RelativePos01 = 1.0f - (((float)(*Value) - Min) / (float)Range);

		float CursorWidth = WorkRectDim.x;
		float CursorHeight = CursorWidth * 0.66f;

		float CursorX = WorkRectMin.x - (CursorWidth - WorkRectDim.x) * 0.5f;
		float CursorY = WorkRectMin.y + (WorkRectDim.y - CursorHeight) * RelativePos01;

		v2 CursorDim = V2(CursorWidth, CursorHeight);
		rect2 CursorRect = Rect2MinDim(V2(CursorX, CursorY), CursorDim);

		float MaxWidth = Max(Max(Max(WorkRectDim.x, MaxValueRcDim.x), MinValueRcDim.x), SmallTextRcDim.x);


		//NOTE(DIMA): Processing interactions
		v4 CursorColor = GUIGetColor(State, State->ColorTheme.SecondaryColor);
		b32 IsHot = GUIInteractionIsHot(State, Interaction);
		b32 MouseInWorkRect = MouseInRect(State->Input, WorkRect);
		b32 MouseInCursRect = MouseInRect(State->Input, CursorRect);
		if (MouseInWorkRect || MouseInCursRect) {
			if (MouseButtonWentDown(State->Input, MouseButton_Left) && !IsHot) {
				GUISetInteractionHot(State, Interaction, true);
				IsHot = true;
			}
		}

		if (MouseInCursRect && !IsHot) {
			char ValStr[64];
			stbsp_sprintf(ValStr, "%.3f", *Value);

			GUILabel(State, ValStr, State->Input->MouseP);
		}

		if (MouseButtonWentUp(State->Input, MouseButton_Left) && IsHot) {
			GUISetInteractionHot(State, Interaction, false);
			IsHot = false;
		}

		if (GUIWalkaroundIsHere(State)) {

			if (ButtonWentDown(State->Input, KeyType_Return)) {
				GUISwapWalkaroundHot(State);
			}

			if (GUIWalkaroundIsHot(State)) {
				PushRectOutline(State->RenderStack, WorkRect, 2, GUIGetColor(State, State->ColorTheme.WalkaroundHotColor));

				CursorColor = GUIGetColor(State, State->ColorTheme.TextHighlightColor);

				float ChangeStep = 0.02f * Range;
				if (ButtonIsDown(State->Input, KeyType_Down)) {
					float NewValue = *Value - ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				if (ButtonIsDown(State->Input, KeyType_Up)) {
					float NewValue = *Value + ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				char ValStr[16];
				stbsp_sprintf(ValStr, "%.3f", *Value);
				GUILabel(State, ValStr, CursorRect.Max);
			}
			else {
				//PushRectOutline(State->RenderStack, Rect2MinDim(MaxValueRcMin, V2(MaxWidth, SmallTextRc.Max.y - MaxValueRc.Min.y)));
				PushRectOutline(State->RenderStack, SmallTextRc, 2, GUIGetColor(State, State->ColorTheme.TextHighlightColor));
				PushRectOutline(State->RenderStack, WorkRect, 2, GUIGetColor(State, State->ColorTheme.TextHighlightColor));
			}
		}

		if (IsHot) {
			CursorColor = GUIGetColor(State, State->ColorTheme.TextHighlightColor);

			v2 InteractMouseP = State->Input->MouseP;
			if (InteractMouseP.y > (WorkRect.Min.y - 0.5f * CursorHeight)) {
				*Value = Max;
			}

			if (InteractMouseP.y < (WorkRect.Max.y + 0.5f * CursorHeight)) {
				*Value = Min;
			}

			float AT = InteractMouseP.y - (WorkRect.Min.y + 0.5f * CursorHeight);
			AT = Clamp(AT, 0.0f, WorkRectDim.y - CursorHeight);
			float NewVal01 = 1.0f - (AT / (WorkRectDim.y - CursorHeight));
			float NewValue = Min + NewVal01 * Range;
			*Value = NewValue;

			char ValStr[16];
			stbsp_sprintf(ValStr, "%.3f", *Value);
			GUILabel(State, ValStr, CursorRect.Max);
		}

		//NOTE(DIMA): Drawing cursor
		PushRect(State->RenderStack, CursorRect, CursorColor);
		PushRectOutline(State->RenderStack, CursorRect, 2, GUIGetColor(State, State->ColorTheme.OutlineColor));

		//NOTE(DIMA): Postprocessing
		GUIDescribeElementDim(
			State,
			V2(MaxWidth,
				SmallTextRc.Max.y - MaxValueRc.Min.y));

		GUIAdvanceCursor(State);
	}

	GUIEndElement(State, GUIElement_InteractibleItem);
}

void GUISlider(gui_state* GUIState, char* Name, float Min, float Max, gui_interaction* Interaction) {
	b32 NeedShow = GUIBeginElement(GUIState, GUIElement_InteractibleItem, Name, Interaction);

	if (NeedShow) {
		gui_view* View = GUIGetCurrentView(GUIState);

		GUIPreAdvanceCursor(GUIState);

		float NextRowAdvanceFull = GetNextRowAdvance(GUIState->FontInfo, GUIState->FontScale);

		float* Value = Interaction->VariableLink.Value_F32;

		Assert(Max > Min);

		rect2 NameTextSize = PrintTextInternal(
			GUIState,
			PrintTextType_PrintText,
			Name,
			View->CurrentX,
			View->CurrentY,
			GUIState->FontScale,
			GUIGetColor(GUIState, GUIState->ColorTheme.TextColor));
		v2 NameTextDim = GetRectDim(NameTextSize);

		char ValueBuf[32];
		stbsp_sprintf(ValueBuf, "%.3f", *Value);
		rect2 ValueTextSize = PrintTextInternal(GUIState, PrintTextType_GetTextSize, ValueBuf, 0, 0, GUIState->FontScale);

		//NOTE(Dima): Next element to the text is advanced by AscenderHeight
		v2 WorkRectMin = V2(
			View->CurrentX + NameTextDim.x + GUIState->FontInfo->AscenderHeight * GUIState->FontScale,
			View->CurrentY - GUIState->FontInfo->AscenderHeight * GUIState->FontScale);

		v2 WorkRectDim = V2(NextRowAdvanceFull * 10, NextRowAdvanceFull);

		rect2 WorkRect = Rect2MinDim(WorkRectMin, WorkRectDim);
		v4 WorkRectColor = GUIGetColor(GUIState, GUIState->ColorTheme.FirstColor);

		float RectOutlineWidth = 1.0f;

		PushRect(GUIState->RenderStack, WorkRect, WorkRectColor);
		PushRectOutline(GUIState->RenderStack, WorkRect, RectOutlineWidth, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		float Range = Max - Min;
		if (*Value > Max) {
			*Value = Max;
		}
		else if (*Value < Min) {
			*Value = Min;
		}

		float RelativePos01 = ((float)(*Value) - Min) / (float)Range;

		float CursorHeight = WorkRectDim.y;
		float CursorWidth = CursorHeight * 0.75f;

		float CursorX = WorkRectMin.x + (WorkRectDim.x - CursorWidth) * RelativePos01;
		float CursorY = WorkRectMin.y - (CursorHeight - WorkRectDim.y) * 0.5f;

		v2 CursorDim = V2(CursorWidth, CursorHeight);
		rect2 CursorRect = Rect2MinDim(V2(CursorX, CursorY), CursorDim);
		v4 CursorColor = GUIGetColor(GUIState, GUIState->ColorTheme.SecondaryColor);

		b32 IsHot = GUIInteractionIsHot(GUIState, Interaction);

		if (MouseInRect(GUIState->Input, CursorRect) || MouseInRect(GUIState->Input, WorkRect)) {

			if (MouseButtonWentDown(GUIState->Input, MouseButton_Left) && !IsHot) {
				GUISetInteractionHot(GUIState, Interaction, true);
				IsHot = true;
			}
		}

		if (MouseButtonWentUp(GUIState->Input, MouseButton_Left) && IsHot) {
			GUISetInteractionHot(GUIState, Interaction, false);
			IsHot = false;
		}

		if (IsHot) {
			CursorColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);

			v2 InteractMouseP = GUIState->Input->MouseP;
#if 0
			if (InteractMouseP.x > (WorkRect.Max.x - 0.5f * CursorWidth)) {
				*Value = Max;
				InteractMouseP.x = (WorkRect.Max.x - 0.5f * CursorWidth);
			}

			if (InteractMouseP.x < (WorkRect.Min.x + 0.5f * CursorWidth)) {
				*Value = Min;
				InteractMouseP.x = (WorkRect.Min.x + 0.5f * CursorWidth);
			}
#endif

			float AT = InteractMouseP.x - (WorkRect.Min.x + 0.5f * CursorWidth);
			AT = Clamp(AT, 0.0f, WorkRectDim.x - CursorWidth);
			float NewVal01 = AT / (WorkRectDim.x - CursorWidth);
			float NewValue = Min + NewVal01 * Range;
			*Value = NewValue;
		}

		if (GUIWalkaroundIsHere(GUIState)) {
			if (ButtonWentDown(GUIState->Input, KeyType_Return)) {
				GUISwapWalkaroundHot(GUIState);
			}

			if (GUIState->WalkaroundIsHot) {
				PushRectOutline(GUIState->RenderStack, WorkRect, 2, GUIGetColor(GUIState, GUIState->ColorTheme.WalkaroundHotColor));

				CursorColor = GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor);

				float ChangeStep = Range * 0.02f;
				if (ButtonIsDown(GUIState->Input, KeyType_Right)) {
					float NewValue = *Value + ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}

				if (ButtonIsDown(GUIState->Input, KeyType_Left)) {
					float NewValue = *Value - ChangeStep;
					NewValue = Clamp(NewValue, Min, Max);
					*Value = NewValue;
				}
			}
			else {
				PushRectOutline(GUIState->RenderStack, NameTextSize, 2, GUIGetColor(GUIState, GUIState->ColorTheme.TextHighlightColor));
			}
		}

		PushRect(GUIState->RenderStack, CursorRect, CursorColor);
		PushRectOutline(GUIState->RenderStack, CursorRect, 2, GUIGetColor(GUIState, GUIState->ColorTheme.OutlineColor));

		float ValueTextY = WorkRectMin.y + GUIState->FontInfo->AscenderHeight * GUIState->FontScale;
		float ValueTextX = WorkRectMin.x + WorkRectDim.x * 0.5f - (ValueTextSize.Max.x - ValueTextSize.Min.x) * 0.5f;
		PrintTextInternal(GUIState, PrintTextType_PrintText, ValueBuf, ValueTextX, ValueTextY, GUIState->FontScale);

#if 0
		char TextBuf[64];
		stbsp_snprintf(TextBuf, sizeof(TextBuf), "Min: %.3f; Max: %.3f;", Min, Max);

		float DrawTextX = View->CurrentX + WorkRectMin.x + WorkRectDim.x + 10;
		PrintTextInternal(GUIState, PrintTextType_PrintText, TextBuf, DrawTextX, View->CurrentY, View->FontScale);
#endif

		GUIDescribeElementDim(
			GUIState,
			V2(WorkRect.Max.x - View->CurrentX,
				WorkRect.Max.y - WorkRect.Min.y + (2.0f * RectOutlineWidth)));

		GUIAdvanceCursor(GUIState);
	}

	GUIEndElement(GUIState, GUIElement_InteractibleItem);
}

void GUITreeBegin(gui_state* State, char* NodeName) {
	gui_view* View = GUIGetCurrentView(State);

	b32 NeedShow = GUIBeginElement(State, GUIElement_TreeNode, NodeName, 0);
	gui_element* Elem = GUIGetCurrentElement(State);
	gui_tree_node_cache* Cache = &Elem->Cache.TreeNode;

	gui_interaction ActionInteraction = GUIVariableInteraction(&State->CurrentNode->Expanded, GUIVarType_B32);
	char TextBuf[64];
	if (State->PlusMinusSymbol) {
		stbsp_sprintf(TextBuf, "%c %s", Elem->Expanded ? '+' : '-', Elem->Text);
	}
	else {
		stbsp_sprintf(TextBuf, "%s", Elem->Text);
	}

	if (NeedShow) {
		GUIPreAdvanceCursor(State);

		//gui_interaction Interaction_ = GUIVariableInteraction(&State->CurrentNode->Expanded, GUIVarType_B32);
		//gui_interaction* Interaction = &Interaction_;

		rect2 Rc = PrintTextInternal(State, PrintTextType_GetTextSize, TextBuf, View->CurrentX, View->CurrentY, State->FontScale);
		v2 Dim = V2(Rc.Max.x - Rc.Min.x, Rc.Max.y - Rc.Min.y);

		v4 TextHighlightColor = GUIGetColor(State, State->ColorTheme.TextColor);
		if (MouseInRect(State->Input, Rc)) {
			TextHighlightColor = GUIGetColor(State, State->ColorTheme.TextHighlightColor);
			if (MouseButtonWentDown(State->Input, MouseButton_Left)) {
				Elem->Expanded = !Elem->Expanded;
				Cache->ExitState = 0;
			}
		}

		if (GUIWalkaroundIsHere(State)) {
			PushRectOutline(State->RenderStack, Rc, 2, GUIGetColor(State, State->ColorTheme.TextHighlightColor));
		}

		PrintTextInternal(State, PrintTextType_PrintText, TextBuf, View->CurrentX, View->CurrentY, State->FontScale, TextHighlightColor);

		//NOTE(Dima): Remember last element width for BeginRow/EndRow
		GUIDescribeElementDim(State, GetRectDim(Rc));

		GUIAdvanceCursor(State);

	}
	Elem->Cache.TreeNode.StackBeginY = View->CurrentY;
}

void GUITreeEnd(gui_state* State) {
	gui_element* Elem = GUIGetCurrentElement(State);
	gui_view* View = GUIGetCurrentView(State);

	gui_tree_node_cache* Cache = &Elem->Cache.TreeNode;

	if ((Elem->Expanded == false) &&
		(Cache->ExitState == GUITreeNodeExit_None))
	{
		Cache->ExitY = Cache->StackY - Cache->StackBeginY;
		//Cache->ExitY = 100;
		Cache->ExitState = GUITreeNodeExit_Exiting;
	}

	if (Cache->ExitState == GUITreeNodeExit_Exiting) {
		float ExitSpeed = Cache->ExitY / 8.0f + 4;
		Cache->ExitY -= ExitSpeed;

		if (Cache->ExitY < 0.0f) {
			Cache->ExitY = 0.0f;
			Cache->ExitState = GUITreeNodeExit_Finished;
		}
		else {
			GUIPreAdvanceCursor(State);
			GUIDescribeElementDim(State, V2(0, Cache->ExitY));
			GUIAdvanceCursor(State);
		}
	}

	GUIEndElement(State, GUIElement_TreeNode);
}

#if 0
static gui_element* GUIWalkaroundBFS(gui_element* At, char* NodeName) {
	gui_element* Result = 0;

	//NOTE(Dima): First - walk through all elements on the level
	for (gui_element* Element = At->ChildrenSentinel->NextBro;
		Element != At->ChildrenSentinel;
		Element = Element->NextBro)
	{
		if (StringsAreEqual(Element->Name, NodeName)) {
			Result = Element;
			return(Result);
		}
	}

	//NOTE(Dima): If nothing found - recursivery go through every children
	for (gui_element* Element = At->ChildrenSentinel->NextBro;
		Element != At->ChildrenSentinel;
		Element = Element->NextBro)
	{
		Result = GUIWalkaroundBFS(Element, NodeName);
		if (Result) {
			return(Result);
		}
	}

	//NOTE(Dima): Return 0 if nothing found
	return(Result);
}

void GUIBeginTreeFind(gui_state* State, char* NodeName) {
	gui_view* View = GUIGetCurrentView(State);

	gui_element* NeededElement = GUIWalkaroundBFS(State->RootNode, NodeName);
	Assert(NeededElement);

	gui_element* OldParent = View->CurrentNode;
	View->CurrentNode = NeededElement->Parent;
	View->CurrentNode->TempParent = OldParent;

	GUITreeBegin(State, NodeName);
}

void GUIEndTreeFind(gui_state* State) {
	gui_view* View = GUIGetCurrentView(State);

	GUITreeEnd(State);

	gui_element* Temp = View->CurrentNode;
	View->CurrentNode = View->CurrentNode->TempParent;
	Temp->TempParent = 0;
}
#endif

