#include "gore_game_settings.h"

//#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define DIMA_JSON_WRITER_USE_STB_SPRINTF
#define DIMA_JSON_WRITER_IMPLEMENTATION
#include "dima_json_writer.h"

inline game_setting* GetNextGameSetting(
	game_settings* Settings,
	char* SettingName,
	u32 SettingType)
{
	game_setting* Result = Settings->Settings + Settings->LastSettingIndex++;

	Assert(Settings->LastSettingIndex < ArrayCount(Settings->Settings));

	CopyStrings(Result->Name, SettingName);
	Result->Type = SettingType;
}

static game_setting* InitGameSettingInt(game_settings* Settings, char* Name, int Value) {
	game_setting* Result = GetNextGameSetting(Settings, Name, GameSetting_Int);

	Result->IntegerValue= Value;
}

static game_setting* InitGameSettingBool(game_settings* Settings, char* Name, b32 Value) {
	game_setting* Result = GetNextGameSetting(Settings, Name, GameSetting_Bool);

	Result->BoolValue = Value;
}

static game_setting* InitGameSettingString(game_settings* Settings, char* Name, char* Value) {
	game_setting* Result = GetNextGameSetting(Settings, Name, GameSetting_String);

	CopyStrings(Result->StringValue, Value);
}

static game_setting* InitGameSettingFloat(game_settings* Settings, char* Name, float Value) {
	game_setting* Result = GetNextGameSetting(Settings, Name, GameSetting_Float);

	Result->FloatValue = Value;
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

game_settings_values DefaultGameSettingsValues() {
	game_settings_values Settings = {};

	Settings.AnisotropicFilterLevelType = AnisoLevel_1x;
	Settings.AntialiasingType = AA_FXAA;
	Settings.VSyncEnabled = 0;

	return(Settings);
}

game_settings TryReadGameSettings() {
	game_settings Settings = {};

	game_settings_values SettingsValues = DefaultGameSettingsValues();

	char FilePath[256];
	ConcatStringsUnsafe(FilePath, "../Data/", GAME_SETTINGS_FILE_NAME);

	platform_read_file_result SettingsFileData = PlatformApi.ReadFile(FilePath);



	PlatformApi.FreeFileMemory(&SettingsFileData);

	return(Settings);
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

		switch (Current->Type)
		{
			case GameSetting_Bool: {
				JSONAddS32(&Writer, Current->Name, Current->BoolValue);
			}break;

			case GameSetting_Float: {
				JSONAddF32(&Writer, Current->Name, Current->FloatValue);
			}break;

			case GameSetting_Int: {
				JSONAddF32(&Writer, Current->Name, Current->IntegerValue);
			}break;

			case GameSetting_String: {
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
