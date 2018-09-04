#include "gore_game_settings.h"

//#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define DIMA_JSON_WRITER_USE_STB_SPRINTF
#define DIMA_JSON_WRITER_IMPLEMENTATION
#include "dima_json_writer.h"

inline game_setting* GetNextGameSetting(
	game_settings* Settings,
	char* SettingName,
	u32 SettingValueType)
{
	game_setting* Result = Settings->Settings + Settings->LastSettingIndex++;

	Assert(Settings->LastSettingIndex < ArrayCount(Settings->Settings));

	CopyStrings(Result->Name, SettingName);
	Result->ValueType = SettingValueType;

	return(Result);
}

static game_setting* InitGameSettingInt(game_settings* Settings, char* Name, int Value) {
	game_setting* Result = GetNextGameSetting(Settings, Name, GameSettingValue_Int);

	Result->IntegerValue= Value;

	return(Result);
}

static game_setting* InitGameSettingBool(game_settings* Settings, char* Name, b32 Value) {
	game_setting* Result = GetNextGameSetting(Settings, Name, GameSettingValue_Bool);

	Result->BoolValue = Value;

	return(Result);
}

static game_setting* InitGameSettingString(game_settings* Settings, char* Name, char* Value) {
	game_setting* Result = GetNextGameSetting(Settings, Name, GameSettingValue_String);

	CopyStrings(Result->StringValue, Value);

	return(Result);
}

static game_setting* InitGameSettingFloat(game_settings* Settings, char* Name, float Value) {
	game_setting* Result = GetNextGameSetting(Settings, Name, GameSettingValue_Float);

	Result->FloatValue = Value;

	return(Result);
}

#define GAME_SETTING_ANISOTROPIC_NAME "AnisotropicLevel"
#define GAME_SETTING_ANTIALIASING_NAME "Antialiasing"
#define GAME_SETTING_VSYNC_NAME "VSync"

static void InitInternalSettingsBasedOnValues(
	game_settings* Settings,
	game_settings_values* Values) 
{
	Settings->LastSettingIndex = 0;

	Settings->AnisotropicLevelSetting = InitGameSettingInt(
		Settings,
		GAME_SETTING_ANISOTROPIC_NAME,
		Values->AnisotropicFilterLevelType);

	Settings->AntialiasingTypeSetting = InitGameSettingInt(
		Settings,
		GAME_SETTING_ANTIALIASING_NAME,
		Values->AntialiasingType);

	Settings->VSyncEnabledSetting = InitGameSettingBool(
		Settings,
		GAME_SETTING_VSYNC_NAME,
		Values->VSyncEnabled);
}

void WriteGameSettings(game_settings* Settings) {
	json_writer Writer;
	JSONInit(&Writer, JSONWriterFlag_Pretty);

	JSONBegin(&Writer);
	for (int SettingIndex = 0;
		SettingIndex < Settings->LastSettingIndex;
		SettingIndex++)
	{
		game_setting* Current = Settings->Settings + SettingIndex;

		switch (Current->ValueType)
		{
			case GameSettingValue_Bool: {
				JSONAddS32(&Writer, Current->Name, Current->BoolValue);
			}break;

			case GameSettingValue_Float: {
				JSONAddF32(&Writer, Current->Name, Current->FloatValue);
			}break;

			case GameSettingValue_Int: {
				JSONAddS32(&Writer, Current->Name, Current->IntegerValue);
			}break;

			case GameSettingValue_String: {
				JSONAddSTR(&Writer, Current->Name, Current->StringValue);
			}break;

			default:
				break;
		}
	}
	JSONEnd(&Writer);

	char FilePath[256];
	ConcatStringsUnsafe(FilePath, "../Data/", GAME_SETTINGS_FILE_NAME);

	PlatformApi.WriteFile(FilePath, Writer.Buf, Writer.CurrentIndex);

	JSONFree(&Writer);
}


game_settings_values DefaultGameSettingsValues() {
	game_settings_values Settings = {};

	Settings.AnisotropicFilterLevelType = AnisoLevel_1x;
	Settings.AntialiasingType = AA_FXAA;
	Settings.VSyncEnabled = 0;

	return(Settings);
}

static void GetNextElementFromInput(char** AtInput, char* ElementBuf, int* ElementLen) {
	int Len = 0;

	char* At = *AtInput;
	char* To = ElementBuf;

	while (*At) {
		if (*At != ',' &&
			*At != ' ' &&
			*At != '\n' &&
			*At != '\r' &&
			*At != ':' &&
			*At != '\"')
		{
			*To++ = *At++;
			Len++;
		}
		else {
			*To = 0;
			*AtInput = At;
			break;
		}
	}
}

static inline b32 CharIsLetter(char c) {
	if ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z'))
	{
		return(1);
	}

	return(0);
}

game_settings TryReadGameSettings() {
	game_settings Settings = {};

	char FilePath[256];
	ConcatStringsUnsafe(FilePath, "../Data/", GAME_SETTINGS_FILE_NAME);

	platform_read_file_result SettingsFileData = PlatformApi.ReadFile(FilePath);

	game_settings Def = {};
	game_settings_values DefVals = DefaultGameSettingsValues();
	InitInternalSettingsBasedOnValues(&Def, &DefVals);

	if (SettingsFileData.Size) {
		
		char* TextFromFile = (char*)SettingsFileData.Data;

		//NOTE(dima): Simple one-nested JSON parser
		char* At = TextFromFile;
		b32 LastElementWasOpenBracket = 0;
		b32 LastElementWasComma = 0;
		//NOTE(dima): VERY BAD CODE. Do not use it!!!!
		//NOTE(dima): It's only used here to extract values from config JSON file
		while (*At) {

			if ((*At >= 'a' && *At <= 'z') ||
				(*At >= 'A' && *At <= 'Z'))
			{
				//NOTE(dima): Get key and value strings
				int KeyLen, ValLen;
				char KeyBuf[64], ValBuf[64];
				if (LastElementWasComma || LastElementWasOpenBracket) {
					GetNextElementFromInput(&At, KeyBuf, &KeyLen);
				}

				while (*At != ':') {
					*At++;
				}
				*At++;
				while (*At == ' ') {
					At++;
				}
				GetNextElementFromInput(&At, ValBuf, &ValLen);

				//NOTE(dima): Find corresponding values in settings and set them
				for (int SettingIndex = 0; 
					SettingIndex < Settings.LastSettingIndex;
					SettingIndex++)
				{
					game_setting* Setting = Settings.Settings + SettingIndex;
					if (StringsAreEqual(KeyBuf, Setting->Name)) {
						switch (Setting->ValueType) {
							case GameSettingValue_Float: {
								Setting->FloatValue = StringToFloat(ValBuf);
							}break;

							case GameSettingValue_Int: {
								Setting->IntegerValue = StringToInteger(ValBuf);
							}break;

							case GameSettingValue_Bool: {
								Setting->IntegerValue = StringToInteger(ValBuf);
							}break;

							case GameSettingValue_String: {
								CopyStrings(Setting->StringValue, ValBuf);
							}break;
						}

						break;
					}
				}

				continue;
			}

			if (*At == '{') {
				LastElementWasOpenBracket = 1;
			}
			else if (*At == ',') {
				LastElementWasComma = 1;
			}

			*At++;
		}
	}

	WriteGameSettings(&Def);

	PlatformApi.FreeFileMemory(&SettingsFileData);

	return(Settings);
}
//
//game_setting* GetGameSetting(game_settings* Settings) {
//	for (int SettingIndex = 0;
//		SettingIndex < Settings->LastSettingIndex;
//		SettingIndex++)
//	{
//
//	}
//}