
#include <psp2/kernel/modulemgr.h>
#include <taihen.h>
#include <string.h>
#include <vitasdk.h>
#include "ranking.h"
#include <stdio.h>
#include <vita2d.h>
#include <inttypes.h>

#define IME_DIALOG_RESULT_NONE 0
#define IME_DIALOG_RESULT_RUNNING 1
#define IME_DIALOG_RESULT_FINISHED 2
#define IME_DIALOG_RESULT_CANCELED 3



static SceUID SysmoduleLoad = -1;
static tai_hook_ref_t SysmoduleLoad_REF;

static SceUID SubmitScore = -1;
static tai_hook_ref_t SubmitScore_REF;

static SceUID SubmitScoreAsync = -1;
static tai_hook_ref_t SubmitScoreAsync_REF;

int ret = 0;

static uint16_t ime_title_utf16[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t ime_initial_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t ime_input_text_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
static uint8_t ime_input_text_utf8[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];

void utf16_to_utf8(uint16_t *src, uint8_t *dst) {
	int i;
	for (i = 0; src[i]; i++) {
		if ((src[i] & 0xFF80) == 0) {
			*(dst++) = src[i] & 0xFF;
		} else if((src[i] & 0xF800) == 0) {
			*(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		} else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
			*(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
			*(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
			*(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
			*(dst++) = (src[i + 1] & 0x3F) | 0x80;
			i += 1;
		} else {
			*(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
			*(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
			*(dst++) = (src[i] & 0x3F) | 0x80;
		}
	}

	*dst = '\0';
}

void utf8_to_utf16(uint8_t *src, uint16_t *dst) {
	int i;
	for (i = 0; src[i];) {
		if ((src[i] & 0xE0) == 0xE0) {
			*(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
			i += 3;
		} else if ((src[i] & 0xC0) == 0xC0) {
			*(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
			i += 2;
		} else {
			*(dst++) = src[i];
			i += 1;
		}
	}

	*dst = '\0';
}
 
void initImeDialog(char *title, char *initial_text, int max_text_length) {
    // Convert UTF8 to UTF16
	utf8_to_utf16((uint8_t *)title, ime_title_utf16);
	utf8_to_utf16((uint8_t *)initial_text, ime_initial_text_utf16);
 
    SceImeDialogParam param;
	sceImeDialogParamInit(&param);

	param.sdkVersion = 0x03150021,
	param.supportedLanguages = 0x0001FFFF;
	param.languagesForced = SCE_TRUE;
	param.type = SCE_IME_TYPE_EXTENDED_NUMBER;
	param.title = ime_title_utf16;
	param.maxTextLength = 20;
	param.initialText = ime_initial_text_utf16;
	param.inputTextBuffer = ime_input_text_utf16;

	//int res = 
	sceImeDialogInit(&param);
	return ;
}

void oslOskGetText(char *text){
	// Convert UTF16 to UTF8
	utf16_to_utf8(ime_input_text_utf16, ime_input_text_utf8);
	strcpy(text,(char*)ime_input_text_utf8);
}

SceInt64 open_ime(SceInt64 score) {
    
	sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
    sceCommonDialogSetConfigParam(&(SceCommonDialogConfigParam){});
 
    vita2d_init();
   
    int shown_dial = 0;
	
	char userText[512];
	memset(userText,0x00,512);
	
	sprintf(userText, "%"PRId64,score);
	
	while (1) {
        if(!shown_dial){
            initImeDialog("Enter Your REAL Score", userText, 128);
			shown_dial=1;
        }

        SceCommonDialogStatus status = sceImeDialogGetStatus();
       
		if (status == IME_DIALOG_RESULT_FINISHED) {
			SceImeDialogResult result;
			memset(&result, 0, sizeof(SceImeDialogResult));
			sceImeDialogGetResult(&result);

			if (result.button == SCE_IME_DIALOG_BUTTON_CLOSE) {
				status = IME_DIALOG_RESULT_CANCELED;
				break;
			} else {
				oslOskGetText(userText);
				SceInt64 newScore;
				sscanf(userText, "%"PRId64, &newScore);
				score = newScore;
				break;
			}

			shown_dial = 0;
			
			
		}
       
        vita2d_common_dialog_update();
        vita2d_swap_buffers();
       
    }
	sceImeDialogTerm();
	
    return score;
}

int	sceNpScoreRecordScoreAsync_patched(SceInt32 reqId,SceNpScoreBoardId boardId,SceNpScoreValue score,const SceNpScoreComment *scoreComment,const SceNpScoreGameInfo *gameInfo,SceNpScoreRankNumber *tmpRank,const SceRtcTick *compareDate,void *option)
{
	sceClibPrintf("[ScoreHax] OldScore %"PRId64"\n",score);
	score = open_ime(score);
	sceClibPrintf("[ScoreHax] NewScore %"PRId64"\n",score);

	int ret = TAI_CONTINUE(int,SubmitScoreAsync_REF,reqId,boardId,score,scoreComment,gameInfo,tmpRank,compareDate,option);
	sceClibPrintf("[ScoreHax] ScoreAsync Returned %x~\n",ret);
	return ret;
}

int	sceNpScoreRecordScore_patched(SceInt32 reqId,SceNpScoreBoardId boardId,SceNpScoreValue score,const SceNpScoreComment *scoreComment,const SceNpScoreGameInfo *gameInfo,SceNpScoreRankNumber *tmpRank,const SceRtcTick *compareDate,void *option)
{
	sceClibPrintf("[ScoreHax] OldScore %"PRId64"\n",score);
	score = open_ime(score);
	sceClibPrintf("[ScoreHax] NewScore %"PRId64"\n",score);

	int ret = TAI_CONTINUE(int,SubmitScore_REF,reqId,boardId,score,scoreComment,gameInfo,tmpRank,compareDate,option);
	sceClibPrintf("[ScoreHax] Score Returned %x~\n",ret);
	return ret;
}



static int sceSysmoduleLoadModule_patched(SceSysmoduleModuleId id) {
	int ret = TAI_CONTINUE(int, SysmoduleLoad_REF, id);
	if(id == SCE_SYSMODULE_NP_SCORE_RANKING)
	{
		sceClibPrintf("[ScoreHax] NP SCORE LOADED! 0x%x\n",ret);
		
		SubmitScore = taiHookFunctionExport
		(&SubmitScore_REF,   // Output a reference
		"SceNpScore",	     // Name of module being hooked
		0x75F052DB,			 // NID specifying SceNpScore
		0x320C0277,			 // NID specifying sceNpScoreRecordScore
		sceNpScoreRecordScore_patched);
		sceClibPrintf("[ScoreHax] SubmitScore %x %x~\n",SubmitScore,SubmitScore_REF);
		
		SubmitScoreAsync = taiHookFunctionExport
		(&SubmitScoreAsync_REF,   // Output a reference
		"SceNpScore",	     // Name of module being hooked
		0x75F052DB,			 // NID specifying SceNpScore
		0x24B09634,			 // NID specifying sceNpScoreRecordScoreAsync
		sceNpScoreRecordScoreAsync_patched);
		sceClibPrintf("[ScoreHax] SubmitScore %x %x~\n",SubmitScoreAsync,SubmitScoreAsync_REF);
	}
	return ret;
}



void _start() __attribute__ ((weak, alias ("module_start"))); 
int module_start(SceSize argc, const void *args) {
	sceClibPrintf("[ScoreHax] Blessed Be~\n");
	
	SysmoduleLoad = taiHookFunctionImport
	(&SysmoduleLoad_REF,	 // Output a reference
		TAI_MAIN_MODULE,	 // Name of module being hooked
		0x03FCF19D,			 // NID specifying SceSysmodule
		0x79A0160A,			 // NID specifying sceSysmoduleLoadModule
		sceSysmoduleLoadModule_patched);
		
	sceClibPrintf("[ScoreHax] SysmoduleLoadHook: %x %x~\n",SysmoduleLoad,SysmoduleLoad_REF);
		
	
	return SCE_KERNEL_START_SUCCESS;

}

int module_stop(SceSize argc, const void *args) {

  // release hooks
	if (SysmoduleLoad >= 0) taiHookRelease(SysmoduleLoad, SysmoduleLoad_REF);
	if (SubmitScoreAsync >= 0) taiHookRelease(SubmitScoreAsync, SubmitScoreAsync_REF);
	if (SubmitScore >= 0) taiHookRelease(SubmitScore, SubmitScore_REF);

	
  return SCE_KERNEL_STOP_SUCCESS;
}
