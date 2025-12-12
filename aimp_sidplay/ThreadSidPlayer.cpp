#include "StdAfx.h"

//#include <Windows.h>
//#include "Shlwapi.h"

#include "ThreadSidPlayer.h"
#include "SidInfoImpl.h"
#include "c64roms.h"
#include "CSidplayPlugin.h"


CThreadSidPlayer::CThreadSidPlayer(SidDatabase* sidDatabase): m_tune(0)
{
	m_sidDatabase = sidDatabase;
	m_playerStatus = SP_STOPPED;
	m_playerConfig.playLimitEnabled = false;
	m_playerConfig.playLimitSec = 120;
	m_playerConfig.songLengthsFile = NULL;
	m_playerConfig.useSongLengthFile = false;
	m_playerConfig.useSTILfile = false;
	m_playerConfig.hvscDirectory = NULL;
	m_playerConfig.voiceConfig[0][0] = true;
	m_playerConfig.voiceConfig[0][1] = true;
	m_playerConfig.voiceConfig[0][2] = true;
	m_playerConfig.voiceConfig[1][0] = true;
	m_playerConfig.voiceConfig[1][1] = true;
	m_playerConfig.voiceConfig[1][2] = true;
	m_playerConfig.voiceConfig[2][0] = true;
	m_playerConfig.voiceConfig[2][1] = true;
	m_playerConfig.voiceConfig[2][2] = true;
	m_playerConfig.playlistFormat = new char[32];
	strcpy(m_playerConfig.playlistFormat, "%t %x %sn / %a / %r %st");
	m_playerConfig.subsongFormat = new char[32];
	strcpy(m_playerConfig.subsongFormat, "(Tune %n)");

	m_playerConfig.pseudoStereo = false;
	m_playerConfig.sid2Model = SidConfig::sid_model_t::MOS6581;	
	m_currentTuneLength = -1;
	m_engine = new sidplayfp;
	m_decodedBytesCount = 0;
	m_totalBytesCount = 0;
}

CThreadSidPlayer::~CThreadSidPlayer(void)
{
	ClearSTILData();
	if (m_engine != NULL)
	{
		if (m_playerConfig.sidConfig.sidEmulation != NULL)
		{
			delete m_playerConfig.sidConfig.sidEmulation;
			m_playerConfig.sidConfig.sidEmulation = NULL;
		}
		delete m_engine;
	}
}

void CThreadSidPlayer::Init(void)
{ 

	if(m_playerStatus != SP_STOPPED) Stop();

	m_playerConfig.sidConfig = m_engine->config();

	if(!LoadConfigFromFile(&m_playerConfig))
	{
		//if load fails then use this default settings
		//m_playerConfig.sidConfig.precision = 16;
		SidConfig *defaultConf = new SidConfig;
		memcpy((void*)&(m_playerConfig.sidConfig), defaultConf, sizeof(SidConfig));
		
		delete defaultConf;
		m_playerConfig.sidConfig.frequency = 44100;
		m_playerConfig.sidConfig.playback = SidConfig::MONO;// sid2_mono;
	}
	//m_playerConfig.sidConfig.sampleFormat = SID2_LITTLE_SIGNED;	
	SetConfig(&m_playerConfig);
}

void CThreadSidPlayer::Stop(void)
{
	if(m_playerStatus == SP_STOPPED) return;
	m_playerStatus = SP_STOPPED; //to powinno zatrzymaæ w¹tek
	m_engine->stop();
	m_decodedBytesCount = 0;
	
}

void CThreadSidPlayer::LoadTune(const char* name, int startSong)
{
	Stop();
	m_engine->fastForward(100);
	
	m_tune.load(name);
	const SidTuneInfo* tuneInfo = m_tune.getInfo();
	if (tuneInfo == NULL)
	{
		return;
	}
	if (startSong == -1)
	{
		m_tune.selectSong(m_tune.getInfo()->startSong());
	}
	else
	{
		m_tune.selectSong(startSong);
	}
	m_currentTuneLength = m_sidDatabase->length(m_tune);
	if ((m_playerConfig.playLimitEnabled) && (m_currentTuneLength <= 0))
	{
		m_currentTuneLength = m_playerConfig.playLimitSec;
	}
	m_engine->load(&m_tune);
	m_engine->initMixer(m_playerConfig.sidConfig.playback == SidConfig::playback_t::STEREO);
	m_engine->reset();
 
	//mute must be applied after SID's have been created
	for (int sid = 0; sid < 3; ++sid)
	{
		for (int voice = 0; voice < 3; ++voice)
		{
			m_engine->mute(sid, voice, !m_playerConfig.voiceConfig[sid][voice]);
		}
	}

	m_decodedBytesCount = 0;

	//sidconfig.playback is enum with values 1 for mono and 2 for stereo, all is multipliet by 2 to have bytes instead of 16bit sample count
	m_totalBytesCount = m_currentTuneLength * m_playerConfig.sidConfig.frequency * m_playerConfig.sidConfig.playback * 2; 
	m_playerStatus = PlayerStatus_t::SP_RUNNING;
}

UINT64 CThreadSidPlayer::GetTotalBytesCount() { 
	return m_totalBytesCount; }
UINT64 CThreadSidPlayer::GetDecodedBytesCount() { 
	return m_decodedBytesCount; }


int CThreadSidPlayer::CurrentSubtune(void)
{
	if(m_tune.getStatus()) 
		return m_tune.getInfo()->currentSong();
	return 0;
}

void CThreadSidPlayer::PlaySubtune(int subTune)
{	
	Stop();
	m_tune.selectSong(subTune);	
	m_currentTuneLength = m_sidDatabase->length(m_tune);
	if ((m_playerConfig.playLimitEnabled) && (m_currentTuneLength <= 0))
	{
		m_currentTuneLength = m_playerConfig.playLimitSec;
	}
	m_engine->stop();
	m_engine->load(&m_tune);
	m_decodedBytesCount = 0;

	m_engine->initMixer(m_playerConfig.sidConfig.playback == SidConfig::playback_t::STEREO);
	m_engine->reset();
	for (int sid = 0; sid < 3; ++sid)
	{
		for (int voice = 0; voice < 3; ++voice)
		{
			m_engine->mute(sid, voice, !m_playerConfig.voiceConfig[sid][voice]);
		}
	}
	m_playerStatus = PlayerStatus_t::SP_RUNNING;
}

const SidTuneInfo* CThreadSidPlayer::GetTuneInfo(void)
{
	return (m_tune.getStatus()) ? m_tune.getInfo() : NULL; //SidTuneInfo();
}

bool CThreadSidPlayer::LoadConfigFromFile(PlayerConfig *conf)
{
	const wchar_t *CONFIG_FOLDER = L"\\aimp_sidplay";
	wchar_t appDataPath[MAX_PATH];
	if(conf == NULL) return false;
	//try to load config from common file
	SHGetSpecialFolderPath(NULL,appDataPath,CSIDL_COMMON_APPDATA,0);

	//PathFileExists(CONFIG_FOLDER);
	wcscat(appDataPath,L"\\aimp_sidplay.ini");
	return LoadConfigFromFile(conf, appDataPath);
}

bool CThreadSidPlayer::LoadConfigFromFile(PlayerConfig *conf, wchar_t* fileName)
{
	char cLine[200+MAX_PATH];
	int maxLen = 200+MAX_PATH;
	string sLine; 
	string token;
	string value;
	int pos;
	FILE *cfgFile = NULL;

	cfgFile = _wfopen(fileName,L"rb");

	if(cfgFile == NULL) return false;
	while(feof(cfgFile) == 0)
	{
		//file>>cLine; 
		ReadLine(cLine,cfgFile,maxLen);
		if(strlen(cLine) == 0) continue;
		sLine.assign(cLine);
		pos = sLine.find("=");
		token = sLine.substr(0,pos);
		value = sLine.substr(pos+1);
		if((token.length() ==0) || (value.length() ==0)) continue;
		while((value.at(0) == '\"') && (value.at(value.length()-1) != '\"') && (!feof(cfgFile)))
		{
			ReadLine(cLine,cfgFile,maxLen);
			sLine.append(cLine);
		}
		if((value.at(0) == '\"') && (value.at(value.length()-1) == '\"'))
			value = value.substr(1,value.length() -2);
		AssignConfigValue(conf, token, value);
	}
	fclose(cfgFile);
	return true;
}

void CThreadSidPlayer::ReadLine(char* buf,FILE *file,const int maxBuf)
{
	char c;
	size_t readCount;
	int pos =0;

	do
	{
		readCount = fread(&c,1,1,file);
		if(readCount ==0) break;
		if((c != '\r') && (c != '\n')) buf[pos++]=c;
		if(pos == maxBuf) break;
	}
	while(c != '\n');
	buf[pos]='\0';
}


void CThreadSidPlayer::SaveConfigToFile(PlayerConfig *conf)
{
	wchar_t appDataPath[MAX_PATH];
	//try to load config from common file
	//SHGetSpecialFolderPath(NULL,appDataPath,CSIDL_COMMON_APPDATA,0);

	if (SUCCEEDED(SHGetFolderPath(NULL,
		CSIDL_COMMON_APPDATA,
		NULL,
		0,
		appDataPath)))
	{
		wcscat(appDataPath, L"\\aimp_sidplay.ini");
		SaveConfigToFile(conf, appDataPath);
	}

}

void CThreadSidPlayer::SaveConfigToFile(PlayerConfig *plconf, wchar_t* fileName)
{
	SidConfig* conf = &plconf->sidConfig;
	ofstream outFile(fileName);
	outFile << "PlayFrequency=" << conf->frequency << endl;
	outFile << "PlayChannels=" << ((conf->playback == SidConfig::MONO) ? 1 : 2) << endl;
	outFile << "C64Model=" << conf->defaultC64Model << endl;
	outFile << "C64ModelForced=" << conf->forceC64Model << endl;
	outFile << "SidModel=" << conf->defaultSidModel << endl;
	outFile << "SidModelForced=" << conf->forceSidModel << endl;
	outFile << "Sid2ModelForced=" << conf->forceSecondSidModel << endl;
	
	outFile<<"PlayLimitEnabled="<<plconf->playLimitEnabled<<endl;
	outFile<<"PlayLimitTime="<<plconf->playLimitSec<<endl;
	outFile<<"UseSongLengthFile="<<plconf->useSongLengthFile<<endl;
	if((!plconf->useSongLengthFile)||(plconf->songLengthsFile == NULL)) outFile<<"SongLengthsFile="<<""<<endl;
	else outFile<<"SongLengthsFile="<<plconf->songLengthsFile<<endl;
	outFile<<"UseSTILFile="<<plconf->useSTILfile<<endl;
	if(plconf->hvscDirectory == NULL) plconf->useSTILfile = false;
	if(!plconf->useSTILfile) outFile<<"HVSCDir="<<""<<endl;
	else outFile<<"HVSCDir="<<plconf->hvscDirectory<<endl;
	outFile << "UseSongLengthFile=" << plconf->useSongLengthFile << endl;

	outFile << "VoiceConfig=";
	for (int sid = 0; sid < 3; ++sid)
	{
		for (int voice = 0; voice < 3; ++voice)
		{
			outFile << plconf->voiceConfig[sid][voice];
		}
	}
	outFile << endl;
	outFile << "PseudoStereo=" << plconf->pseudoStereo << endl;
	outFile << "Sid2Model=" << plconf->sid2Model << endl;
	outFile << "PlaylistFormat=" << plconf->playlistFormat << endl;
	outFile << "SubsongFormat=" << plconf->subsongFormat << endl;
	outFile.close();
}

void CThreadSidPlayer::AssignConfigValue(PlayerConfig* plconf,string token, string value)
{

	SidConfig* conf = &plconf->sidConfig;
	if(token.compare("PlayFrequency") == 0) { conf->frequency = atoi(value.c_str()); return; }
	if(token.compare("PlayChannels") == 0) 
	{
		if(value.compare("1") == 0)
		{
			conf->playback = SidConfig::MONO;
		}
		else
		{
			conf->playback = SidConfig::STEREO;
		}
		return;
	}
	if(token.compare("C64Model") == 0) 
	{
		conf->defaultC64Model = (SidConfig::c64_model_t)atoi(value.c_str());
		return;
	}
	if (token.compare("C64ModelForced") == 0)
	{
		conf->forceC64Model = (bool)atoi(value.c_str());
		return;
	}

	if(token.compare("SidModel") == 0) 
	{
		conf->defaultSidModel = (SidConfig::sid_model_t)atoi(value.c_str());
		return;
	}

	if (token.compare("VoiceConfig") == 0)
	{
		int digitId = 0;
		for (int sid = 0; sid < 3; ++sid)
		{
			for (int voice = 0; voice < 3; ++voice)
			{
				plconf->voiceConfig[sid][voice] = (value.at(digitId++) == '1');
			}
		}
		return;
	}

	if (token.compare("Sid2Model") == 0)
	{
		plconf->sid2Model = (SidConfig::sid_model_t)atoi(value.c_str());
		return;
	}

	if (token.compare("PseudoStereo") == 0)
	{
		plconf->pseudoStereo = (bool)atoi(value.c_str());
		return;
	}

	if (token.compare("SidModelForced") == 0)
	{
		conf->forceSidModel = (bool)atoi(value.c_str());
		return;
	}

	if (token.compare("Sid2ModelForced") == 0)
	{
		conf->forceSecondSidModel = (bool)atoi(value.c_str());
		return;
	}

	if(token.compare("PlayLimitEnabled") == 0) 
	{
		plconf->playLimitEnabled = (bool)atoi(value.c_str());
		return;
	}
	if(token.compare("PlayLimitTime") == 0) 
	{
		plconf->playLimitSec = atoi(value.c_str());
		return;
	}

	if(token.compare("UseSongLengthFile") == 0)
	{
		plconf->useSongLengthFile =(bool)atoi(value.c_str());
		return;
	}
	if(token.compare("SongLengthsFile") == 0)
	{
		plconf->songLengthsFile = new char[value.length()+1];

		strcpy(plconf->songLengthsFile,value.c_str());
		return;
	}

	if(token.compare("HVSCDir") == 0)
	{
		plconf->hvscDirectory = new char[value.length()+1];
		strcpy(plconf->hvscDirectory,value.c_str());
		return;
	}
	if(token.compare("UseSTILFile") == 0)
	{
		plconf->useSTILfile =(bool)atoi(value.c_str());
		return;
	}

	if (token.compare("PlaylistFormat") == 0)
	{
		//plconf->playlistFormat = new char[value.length() + 1];
		//strcpy(plconf->playlistFormat, value.c_str());
		plconf->playlistFormat = _strdup(value.c_str());
		return;
	}

	if (token.compare("SubsongFormat") == 0)
	{
		//plconf->playlistFormat = new char[value.length() + 1];
		//strcpy(plconf->subsongFormat, value.c_str());
		plconf->subsongFormat = _strdup(value.c_str());
		return;
	}
}

const PlayerConfig& CThreadSidPlayer::GetCurrentConfig()
{
	return m_playerConfig;

}

void CThreadSidPlayer::SetConfig(PlayerConfig* newConfig)
{
	int numChann;
	bool openRes;

	if(m_playerStatus != SP_STOPPED) Stop();
	m_engine->stop();

	sidbuilder* currentBuilder = m_playerConfig.sidConfig.sidEmulation;
	if (m_playerConfig.sidConfig.sidEmulation != NULL)
	{
		//delete m_playerConfig.sidConfig.sidEmulation;
	}
	m_playerConfig.sidConfig.sidEmulation = nullptr;
	m_engine->config(m_playerConfig.sidConfig);

	if (currentBuilder != NULL)
	{
		delete currentBuilder;
	}

	//change assign to memcpy !
	m_playerConfig.sidConfig.frequency = newConfig->sidConfig.frequency;
	m_playerConfig.sidConfig.playback = newConfig->sidConfig.playback;
	m_playerConfig.sidConfig.defaultC64Model = newConfig->sidConfig.defaultC64Model;
	m_playerConfig.sidConfig.forceC64Model = newConfig->sidConfig.forceC64Model;
	m_playerConfig.sidConfig.defaultSidModel = newConfig->sidConfig.defaultSidModel;
	m_playerConfig.sidConfig.forceSidModel = newConfig->sidConfig.forceSidModel;
	m_playerConfig.sidConfig.forceSecondSidModel = newConfig->sidConfig.forceSecondSidModel;
	m_playerConfig.sidConfig.secondSidModel = newConfig->sidConfig.secondSidModel;
	


	m_playerConfig.playLimitEnabled = newConfig->playLimitEnabled;
	m_playerConfig.playLimitSec = newConfig->playLimitSec;
	m_playerConfig.useSongLengthFile = newConfig->useSongLengthFile;

	m_playerConfig.sidConfig.samplingMethod = SidConfig::INTERPOLATE; //RESAMPLE_INTERPOLATE

	m_playerConfig.pseudoStereo = newConfig->pseudoStereo;
	m_playerConfig.sid2Model = newConfig->sid2Model;

	//string memory cannot overlap !!!
	if(m_playerConfig.songLengthsFile != newConfig->songLengthsFile)
	{
		if(m_playerConfig.songLengthsFile != NULL) 
		{
			delete[] m_playerConfig.songLengthsFile;
			m_playerConfig.songLengthsFile = NULL;
		}
		if(newConfig->songLengthsFile != NULL)
		{
			m_playerConfig.songLengthsFile = new char[strlen(newConfig->songLengthsFile)+1];
			strcpy(m_playerConfig.songLengthsFile, newConfig->songLengthsFile);
		}
	}

	m_playerConfig.useSTILfile = newConfig->useSTILfile;
	if(m_playerConfig.hvscDirectory != newConfig->hvscDirectory)
	{
		if(m_playerConfig.hvscDirectory != NULL) 
		{
			delete[] m_playerConfig.hvscDirectory;
			m_playerConfig.hvscDirectory = NULL;
		}
		if(newConfig->hvscDirectory != NULL)
		{
			m_playerConfig.hvscDirectory = new char[strlen(newConfig->hvscDirectory)+1];
			strcpy(m_playerConfig.hvscDirectory, newConfig->hvscDirectory);
		}
	}

	if (newConfig->playlistFormat != m_playerConfig.playlistFormat)
	{
		if (m_playerConfig.playlistFormat != NULL)
		{
			delete[] m_playerConfig.playlistFormat;
			m_playerConfig.playlistFormat = NULL;
		}
		m_playerConfig.playlistFormat = _strdup(newConfig->playlistFormat);//new char[strlen(newConfig->playlistFormat) + 1];
		//strcpy(m_playerConfig.playlistFormat, newConfig->playlistFormat);
	}

	if (newConfig->subsongFormat != m_playerConfig.subsongFormat)
	{
		if (m_playerConfig.subsongFormat != NULL)
		{
			delete[] m_playerConfig.subsongFormat;
			m_playerConfig.subsongFormat = NULL;
		}
		m_playerConfig.subsongFormat = _strdup(newConfig->subsongFormat);//new char[strlen(newConfig->subsongFormat) + 1];
		//strcpy(m_playerConfig.subsongFormat, newConfig->subsongFormat);
	}

	//m_sidBuilder = ReSIDBuilderCreate("");
    //SidLazyIPtr<IReSIDBuilder> rs(m_sidBuilder);

	ReSIDfpBuilder* rs = new ReSIDfpBuilder("ReSIDfp");
	
    if (rs)
    {		
		
		//const SidInfoImpl* si = reinterpret_cast<const SidInfoImpl*>(&m_engine->info());
		
		m_playerConfig.sidConfig.sidEmulation = rs;
		rs->create((m_engine->info()).maxsids());
		rs->filter6581Curve(0.5);
		rs->filter8580Curve(0.5);
		rs->filter6581Range(0.5);
		rs->combinedWaveformsStrength(SidConfig::sid_cw_t::AVERAGE);
		//rs->combinedWaveformsStrength(SidConfig::sid_cw_t::STRONG);
		//filter always enabled
		rs->filter(true);
	}

	//TO CHANGE !!!!!!!
	if (m_playerConfig.pseudoStereo)
	{
		m_playerConfig.sidConfig.secondSidAddress = 0xD400;
		m_playerConfig.sidConfig.secondSidModel = m_playerConfig.sid2Model;
	}
	else
	{
		m_playerConfig.sidConfig.secondSidAddress = 0;
		m_playerConfig.sidConfig.secondSidModel = SidConfig::sid_model_t::MOS8580;
	}
	//kernal,basic,chargen
	m_engine->setRoms(KERNAL_ROM, BASIC_ROM, CHARGEN_ROM);
	m_engine->config(m_playerConfig.sidConfig);
	m_engine->initMixer(m_playerConfig.sidConfig.playback == SidConfig::playback_t::STEREO);
	//m_engine->reset();

	for (int sid = 0; sid < 3; ++sid)
	{
		for (int voice = 0; voice < 3; ++voice)
		{
			m_playerConfig.voiceConfig[sid][voice] = newConfig->voiceConfig[sid][voice];
		}
	}
}

int CThreadSidPlayer::GetSongLength(SidTune &tune)
{
	int length;

	if (tune.getStatus() == false)
	{
		//MessageBoxA(NULL, "Tune status invalid", "Error", MB_OK);
		return -1;
	}

	length = m_sidDatabase->length(tune);
	if ((m_playerConfig.playLimitEnabled) && (length <= 0))
	{
		length = m_playerConfig.playLimitSec;
	}
	return length;
}

int CThreadSidPlayer::GetSongLength()
{
	return m_currentTuneLength;
}

void CThreadSidPlayer::DoSeek(UINT64 newPos)
{
	int bits;
	int skip_bytes;
	int bps = PLAYBACK_BIT_PRECISION;// m_playerConfig.sidConfig.precision;
	int numChn = m_playerConfig.sidConfig.playback;
	int freq = m_playerConfig.sidConfig.frequency;
	int decodedLen = 0;
	const int TEMP_BUF_LEN = 8192;
	byte tmpDecodeBuf[TEMP_BUF_LEN];

	if (newPos <= m_decodedBytesCount)
	{
		//seek time is less than current time - we have to rewind song
		SidTuneInfo *si;

		m_tune.selectSong(m_tune.getInfo()->currentSong());
		//m_currentTuneLength = m_sidDatabase.length(m_tune);//we know length of tune already
		m_engine->stop();
		m_engine->load(&m_tune);
		m_engine->initMixer(m_playerConfig.sidConfig.playback == SidConfig::playback_t::STEREO);
		m_engine->reset();	//timers are now 0
		skip_bytes = newPos;
		m_decodedBytesCount = 0;
	}
	else
	{
		skip_bytes = newPos - m_decodedBytesCount;
	}


	int stereoMultiply = (m_playerConfig.sidConfig.playback == SidConfig::playback_t::STEREO) ? 2 : 1;
	//convert seek byte count to milisecond offsett
	UINT32 seekMs = skip_bytes / sizeof(short) / (freq /1000) / stereoMultiply;
	//naow we have to add seek count to total miliseconds time
	seekMs += m_engine->timeMs();

	const int CYCLE_COUNT = 5000;
	m_engine->fastForward(3200);

	while (m_engine->timeMs() < seekMs)
	{
		auto x = m_engine->timeMs();
		decodedLen += (stereoMultiply * sizeof(short) * m_engine->play(CYCLE_COUNT));
	}
	//update decoded bytes counter
	m_decodedBytesCount += decodedLen;//m_engine->time() * freq * stereoMultiply * sizeof(short);

	m_engine->fastForward(100);
}

void CThreadSidPlayer::FillSTILData()
{
	const int BUFLEN = 160;
	string strKey;
	string strInfo;
	char buf[BUFLEN];

	m.clear();
	FILE *f;
	strcpy(buf, m_playerConfig.hvscDirectory);
	strcat(buf, "\\documents\\stil.txt");
	f = fopen(buf, "rb+");
	if (f == NULL)
	{
		MessageBoxA(NULL, "Error opening STIL file.\r\nDisable STIL info or choose appropriate HVSC directory", "in_sidplay2", MB_OK);
		return;
	}
	while (feof(f) == 0)
	{
		ReadLine(buf, f, 160);
		strKey.clear();
		strInfo.clear();
		if (buf[0] == '/') //new file block
		{
			strKey.assign(buf);
			FixPath(strKey);//.replace("/","\\");
			ReadLine(buf, f, BUFLEN);
			while (strlen(buf) > 0)
			{
				strInfo.append(buf);
				strInfo.append("\r\n");
				ReadLine(buf, f, BUFLEN);
			}
			m[_strdup(strKey.c_str())] = _strdup(strInfo.c_str());
		}
	}
	fclose(f);
}

void CThreadSidPlayer::FillSTILData2()
{
	const int BUFLEN = 160;
	const char* ARTIST =	" ARTIST:";
	const char* TITLE =		"  TITLE:";
	const char* COMMENT =	"COMMENT:";
	const char* AUTHOR = " AUTHOR:";
	const char* NAME = "   NAME:";
	string strKey;
	//string strInfo;
	string tmpStr;
	char buf[BUFLEN];
	int currentSubsong;
	StilBlock* stillBlock;
	vector<StilBlock*> subsongsInfo;

	m_stillMap2.clear();
	FILE *f;
	strcpy(buf, m_playerConfig.hvscDirectory);
	strcat(buf, "\\documents\\stil.txt");
	f = fopen(buf, "rb+");
	if (f == NULL)
	{
		MessageBoxA(NULL, "Error opening STIL file.\r\nDisable STIL info or choose appropriate HVSC directory", "in_sidplay2", MB_OK);
		return;
	}
	while (feof(f) == 0)
	{
		ReadLine(buf, f, 160);
		strKey.clear();
		if (buf[0] == '/') //new file block
		{
			strKey.assign(buf);
			FixPath(strKey);//.replace("/","\\");
			currentSubsong = 0;

			ReadLine(buf, f, BUFLEN);
			stillBlock = new StilBlock;
			subsongsInfo = m_stillMap2[_strdup(strKey.c_str())];
			subsongsInfo.push_back(NULL);
			while (strlen(buf) > 0)
			{
				tmpStr.assign(buf);
				//check for subsong numer
				if (tmpStr.compare(0, 2, "(#") == 0)
				{
					int newSubsong = atoi(tmpStr.substr(2, tmpStr.length() - 3).c_str());
					//if subsong number is different than 1 then store current info and set subsong number to new value
					if (newSubsong != 1)
					{
						//store current subsong info
						subsongsInfo[currentSubsong] = stillBlock;
						currentSubsong = newSubsong-1;
						//ajust vetor size to number of subsongs
						while (subsongsInfo.size() <= newSubsong)
						{
							subsongsInfo.push_back(NULL);
						}
						stillBlock = new StilBlock;
					}
				}
				//ARTIST
				if (tmpStr.compare(0, strlen(ARTIST), ARTIST) == 0)
				{
					stillBlock->ARTIST = tmpStr.substr(strlen(ARTIST) + 1);
				}
				//TITLE
				if (tmpStr.compare(0, strlen(TITLE), TITLE) == 0)
				{
					if (stillBlock->TITLE.empty())
					{
						stillBlock->TITLE = tmpStr.substr(strlen(TITLE) + 1);
					}
					else
					{
						stillBlock->TITLE.append(",");
						stillBlock->TITLE.append(tmpStr.substr(strlen(TITLE) + 1));
					}
				}
				//AUTHOR
				if (tmpStr.compare(0, strlen(AUTHOR), AUTHOR) == 0)
				{
					stillBlock->AUTHOR = tmpStr.substr(strlen(AUTHOR) + 1);
				}
				//NAME
				if (tmpStr.compare(0, strlen(NAME), NAME) == 0)
				{
					stillBlock->NAME = tmpStr.substr(strlen(NAME) + 1);
				}
				//IGNORE COMMENT
				ReadLine(buf, f, BUFLEN);
			}
			subsongsInfo[currentSubsong] = stillBlock;

			m_stillMap2[_strdup(strKey.c_str())] = subsongsInfo;
		}

	}
	fclose(f);
}

void CThreadSidPlayer::FixPath(string& path)
{
	int i;
	for(i=0; i<path.length();++i)
	{
		if(path[i] == '/') path[i] = '\\';
	}
}

const char* CThreadSidPlayer::GetSTILData(const char* filePath)
{
	map<const char*,char*,ltstr>::iterator i;
	char* stilFileName;

	if((filePath == NULL)||(m_playerConfig.hvscDirectory == NULL)) return NULL;
	if(strlen(filePath) < strlen(m_playerConfig.hvscDirectory)) return NULL;
	stilFileName = new char[strlen(filePath) - strlen(m_playerConfig.hvscDirectory) +1];
	strcpy(stilFileName,&filePath[strlen(m_playerConfig.hvscDirectory)]);
	//i = m.find("aa\\DEMOS\\A-F\\Afterburner.sid");
	i = m.find(stilFileName);
	delete[] stilFileName;
	if(i == m.end())
	{
		return NULL;
	}
	return i->second;
	//if(i == NULL) return;
}

const StilBlock* CThreadSidPlayer::GetSTILData2(const char* filePath, int subsong)
{
	map<const char*, vector<StilBlock*>, ltstr>::iterator i;
	char* stilFileName;

	if ((filePath == NULL) || (m_playerConfig.hvscDirectory == NULL)) return NULL;
	if (strlen(filePath) < strlen(m_playerConfig.hvscDirectory)) return NULL;
	stilFileName = new char[strlen(filePath) - strlen(m_playerConfig.hvscDirectory) + 1];
	strcpy(stilFileName, &filePath[strlen(m_playerConfig.hvscDirectory)]);
	//i = m.find("aa\\DEMOS\\A-F\\Afterburner.sid");
	i = m_stillMap2.find(stilFileName);
	delete[] stilFileName;
	if (i == m_stillMap2.end())
	{
		return NULL;
	}

	if (subsong < i->second.size())
	{
		return i->second[subsong];
	}
	return NULL;
	//if(i == NULL) return;
}

void CThreadSidPlayer::ClearSTILData(void)
{
	map<const char*, char*,ltstr>::iterator it = m.begin();
	while(it != m.end())
	{
		const char *x= it->first;
		const char *y= it->second;
		delete[] it->first;
		delete[] it->second;
		++it;
	}
	m.clear();

	map<const char*, vector<StilBlock*>, ltstr>::iterator it2 = m_stillMap2.begin();
	while (it2 != m_stillMap2.end())
	{
		const char *x = it2->first;
		vector<StilBlock*> y = it2->second;
		delete[] it2->first;
		for (vector<StilBlock*>::iterator it3 = y.begin(); it3 != y.end(); ++it3)
		{
			if ((*it3) != NULL)
			{
				delete *it3;
			}
		}
		y.clear();
		++it2;
	}
	m_stillMap2.clear();

}

/** 
* Decode song data
*/
int CThreadSidPlayer::Decode(void* Buffer, int Count)
{
	/* 
	engine->play() - zwraca liczbê wyprodukowanych sampli dla mono
	engine->mix() - jako parametr przyjmuje ile sampli chcemy ziksowaæ ALE wype³nia i zwraca 2x wiêcej jesli miksowanie jest w trybie stereo (m_engine->initMixer(STEREO))
	wiêc przy wyliczaniu liczby cykli musimy uwzglêdniæ pojemnoœc bufora i tryb miksowania
	
	*/
	if (m_playerStatus == PlayerStatus_t::SP_STOPPED || m_decodedBytesCount >= m_totalBytesCount)
	{
		return 0;
	}

	// 1. Przygotowanie wskaŸników i liczników
	short* bufPtr = reinterpret_cast<short*>(Buffer);
	int bytesPerSample = sizeof(short); // 2 bajty
	int totalShortsToFill = Count / bytesPerSample; // Ca³kowita liczba shortów do zapisania
	int shortsWritten = 0; // Ile ju¿ zapisaliœmy

	bool isStereo = (m_playerConfig.sidConfig.playback == SidConfig::STEREO);
	int channels = isStereo ? 2 : 1;

	// 3. Obliczenie wspó³czynnika Cykle -> Sample
	// Wzór: cycles = samples * (cpu_freq / output_freq)
	const double cpuFreq = m_engine->getCpuFrequency();
	double outputFreq = static_cast<double>(m_playerConfig.sidConfig.frequency);
	double cyclesPerSample = cpuFreq / outputFreq;
	int decodedSamples;
	int requiredCycles = (totalShortsToFill / channels) * cyclesPerSample;

	// Zabezpieczenie przed pêtl¹ nieskoñczon¹ w przypadku b³êdu
	if (outputFreq <= 0) return 0;

	int oneStepCycles = 3000;
	int iterations = requiredCycles / oneStepCycles;

	if (iterations <= 0)
	{
		iterations = 1;
		if (requiredCycles <= 0)
		{
			requiredCycles = 50;
		}
		oneStepCycles = requiredCycles;
	}

	for (int i = 0; i < iterations; ++i)
	{
		decodedSamples = m_engine->play(oneStepCycles);
		// mix() zwraca ile ramek faktycznie zmiksowano
		unsigned int mixedSamples = m_engine->mix(bufPtr, decodedSamples);

		// C. Przesuñ wskaŸniki
		bufPtr += mixedSamples;
		shortsWritten += mixedSamples;
		m_decodedBytesCount += (mixedSamples * sizeof(short));
	}
	return shortsWritten * bytesPerSample;

}

