#pragma once


#include <shlobj.h>
#include <fstream>
#include <stdlib.h>
#include <map>
#include <vector>

#include "residfp.h"
#include "sidplayfp/sidplayfp.h"
#include "sidplayfp/SidTuneInfo.h"
#include "sidplayfp/SidTune.h"
#include "utils/SidDatabase.h"

#include "StilBlock.h"


#include "typesdefs.h"
//#include <hash_map>

#define WM_WA_MPEG_EOF WM_USER+2

#define PLAYBACK_BIT_PRECISION 16

using namespace std;

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

class CThreadSidPlayer
{
private:
	//stdext::hash_map<const char*,char*> m;//,hash<const char*>,eqstr> m;
	map<const char*, char*, ltstr> m;//,hash<const char*>,eqstr> m;
	/**! Map - maps file path to vector of subsongs (usually 1) vector contains stuctures with STILL info:	
	TITLE:
	NAME:
	ARTIST:
	AUTHOR:
	COMMENT:
	*/
	map<const char*, vector<StilBlock*>, ltstr> m_stillMap2;//,hash<const char*>,eqstr> m;

    sidplayfp *m_engine;    
	SidTune m_tune;
	PlayerConfig m_playerConfig;
	PlayerStatus_t m_playerStatus;
	UINT64  m_decodedBytesCount;
	//! Length in bytes
	UINT64 m_totalBytesCount;
	//! Length in seconds
	int m_currentTuneLength;
private:
	void AssignConfigValue(PlayerConfig *conf, string token, string value);
	SidDatabase* m_sidDatabase;
	void ReadLine(char* buf,FILE *file,const int maxBuf);
protected:
	//void DoSeek();
	void FixPath(string& path);
	void FillSTILData();
	void FillSTILData2();
	void ClearSTILData(void);
public:	
	CThreadSidPlayer(SidDatabase* sidDatabase);

	~CThreadSidPlayer(void);
	void Init(void);
	//void Play(void);
	//void Pause(void);
	void Stop(void);
	void LoadTune(const char* name,int startSong = -1);
	PlayerStatus_t GetPlayerStatus() { return m_playerStatus;}
	int CurrentSubtune(void);
	void PlaySubtune(int subTune);
	const SidTuneInfo* GetTuneInfo(void);
	//int GetPlayTime();
	UINT64 GetTotalBytesCount();
	UINT64 GetDecodedBytesCount();


	bool LoadConfigFromFile(PlayerConfig *conf);
	bool LoadConfigFromFile(PlayerConfig *conf, wchar_t* fileName);
	void SaveConfigToFile(PlayerConfig *conf);
	void SaveConfigToFile(PlayerConfig *conf, wchar_t* fileName);
	const PlayerConfig& GetCurrentConfig();
	void SetConfig(PlayerConfig* newConfig);
	int GetSongLength(SidTune &tune);
	int GetSongLength();
	//! Moves emulation time pointer to given time
	//void SeekTo(int timeMs);
	const char* GetSTILData(const char* filePath);
	const StilBlock* GetSTILData2(const char* filePath, int subsong);
	//! Do seek to position given in bytes
	void DoSeek(UINT64 pos);

	/**
	Decode pcm data
	*/
	int Decode(void * Buffer, int Count);

};
