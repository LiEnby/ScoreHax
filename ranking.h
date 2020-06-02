#define SCE_NP_SCORE_GAMEINFO_MAXSIZE		(189)
#define SCE_NP_SCORE_COMMENT_MAXLEN			(63)

typedef	SceUInt32	SceNpScoreBoardId;
typedef SceInt64		SceNpScoreValue;
typedef	SceUInt32	SceNpScoreRankNumber;
typedef	SceInt32		SceNpScorePcId;

typedef	struct SceNpScoreGameInfo{
	SceSize		infoSize;
	SceUInt8	pad[4];
	SceUInt8	data[SCE_NP_SCORE_GAMEINFO_MAXSIZE];
	SceUInt8	pad2[3];
} SceNpScoreGameInfo;

typedef	struct SceNpScoreComment{
	char utf8Comment[SCE_NP_SCORE_COMMENT_MAXLEN+1];
} SceNpScoreComment;
