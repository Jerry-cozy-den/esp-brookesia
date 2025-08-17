I (1871732) ESP_COZE_CHAT: Terminal :{"id":"0c424bc4-d1fc-359b-c5aa-5e64b7ec535f","event_type":"conversation.chat.cancel"}
I (1871837) ESP_COZE_CHAT: Terminal :{"id":"0a88381b-aca2-ee4e-0b60-50c5d91f26d3","event_type":"conversation.chat.cancel"}
I (1871853) ESP_GMF_FILE: No more data, ret: 0
I (1871940) ESP_COZE_CHAT: Terminal :{"id":"06db9a65-59f7-1410-7d12-d8dff928f543","event_type":"conversation.chat.cancel"}
I (1871955) ESP_GMF_FILE: No more data, ret: 0
I (1871955) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c6b35fc, job:0x3c69f53c-aud_simp_dec_proc]
I (1871956) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c8194a4, job:0x3c69f618-rate_cvt_proc]
I (1871956) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c8135f4, job:0x3c69f704-ch_cvt_proc]
I (1871956) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c81881c, job:0x3c69f854-bit_cvt_proc]
I (1871957) ESP_GMF_FILE: CLose, 0x3c69f9a0, pos = 14733/14733
I (1871957) ESP_GMF_TASK: One times job is complete, del[wk:0x3c81055c,ctx:0x3c69f53c, label:aud_simp_dec_close]
I (1871957) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8194a4,ctx:0x3c69f618, label:rate_cvt_close]
I (1871958) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8199f0,ctx:0x3c69f704, label:ch_cvt_close]
I (1871958) ESP_GMF_TASK: One times job is complete, del[wk:0x3c81881c,ctx:0x3c69f854, label:bit_cvt_close]
I (1871958) AUDIO_PROCESSOR: Get State, 4,ESP_AUD_SIMPLE_PLAYER_FINISHED
I (1871959) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 1
I (1872045) ESP_COZE_CHAT: Terminal :{"id":"1c126064-ef1f-7491-6657-91ea770e71cf","event_type":"conversation.chat.cancel"}
I (1872074) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 0
I (1872076) ESP_GMF_FILE: Open, dir:1, uri:file://spiffs/wake_up.mp3
I (1872097) ESP_GMF_FILE: File size: 19917 byte, file position: 0
I (1872097) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0131](operator()): Stop audio: file://spiffs/wake_up.mp3
I (1872097) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8199f0,ctx:0x3c69f53c, label:aud_simp_dec_open]
I (1872099) ESP_GMF_ASMP_DEC: NOTIFY Info, rate: 0, bits: 0, ch: 0 --> rate: 16000, bits: 16, ch: 1
I (1872100) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8199f0,ctx:0x3c69f618, label:rate_cvt_open]
I (1872100) ESP_GMF_TASK: One times job is complete, del[wk:0x3c6b35fc,ctx:0x3c69f704, label:ch_cvt_open]
I (1872101) AUDIO_PROCESSOR: Get info, rate:16000, channels:2, bits:32
I (1872101) AUDIO_PROCESSOR: Get State, 1,ESP_AUD_SIMPLE_PLAYER_RUNNING
I (1872101) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8199f0,ctx:0x3c69f854, label:bit_cvt_open]
I (1872150) ESP_COZE_CHAT: Terminal :{"id":"193e65c1-55ff-ad4c-d3ca-e098c53aa69c","event_type":"conversation.chat.cancel"}
I (1873887) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (1873888) AUDIO_PROCESSOR: VAD_START
I (1874444) ESP_GMF_FILE: No more data, ret: 0
I (1874508) ESP_GMF_FILE: No more data, ret: 0
I (1874508) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c814f58, job:0x3c69f53c-aud_simp_dec_proc]
I (1874508) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c818848, job:0x3c69f618-rate_cvt_proc]
I (1874509) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c7a14b8, job:0x3c69f704-ch_cvt_proc]
I (1874509) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c80819c, job:0x3c69f854-bit_cvt_proc]
I (1874509) ESP_GMF_FILE: CLose, 0x3c69f9a0, pos = 19917/19917
I (1874510) ESP_GMF_TASK: One times job is complete, del[wk:0x3c7a14b8,ctx:0x3c69f53c, label:aud_simp_dec_close]
I (1874510) ESP_GMF_TASK: One times job is complete, del[wk:0x3c814f58,ctx:0x3c69f618, label:rate_cvt_close]
I (1874510) ESP_GMF_TASK: One times job is complete, del[wk:0x3c6b35fc,ctx:0x3c69f704, label:ch_cvt_close]
I (1874510) ESP_GMF_TASK: One times job is complete, del[wk:0x3c6e93e4,ctx:0x3c69f854, label:bit_cvt_close]
I (1874511) AUDIO_PROCESSOR: Get State, 4,ESP_AUD_SIMPLE_PLAYER_FINISHED
I (1874511) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 1
I (1874638) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 0
I (1874639) ESP_GMF_TASK: Waiting to run... [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x0, run:0]
I (1874639) ESP_GMF_TASK: Waiting to run... [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x0, run:0]
I (1875094) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (1875094) AUDIO_PROCESSOR: VAD_END
I (1896722) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (1896722) AUDIO_PROCESSOR: VAD_START
I (1898259) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (1898259) AUDIO_PROCESSOR: VAD_END
I (1898331) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (1898332) AUDIO_PROCESSOR: VAD_START
I (1909201) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (1909201) AUDIO_PROCESSOR: VAD_END
I (1939201) BS:Agent: [coze_chat_app.cpp:0459](recorder_event_callback_fn): wakeup end
I (1939201) AUDIO_PROCESSOR: WAKEUP_END
I (2179726) BS:Agent: [coze_chat_app.cpp:0448](recorder_event_callback_fn): wakeup start
I (2179727) BS:Agent: [coze_chat_app.cpp:0162](change_wakeup_state): change_wakeup_state: 1, force: 0
I (2179727) AUDIO_PROCESSOR: WAKEUP_START [1 : 1]
I (2179728) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0561](processAudioEvent): Play audio: file://spiffs/response_wo_zai_ting_ne.mp3(create_time_ms: 2177701, last_play_time_ms: 0) with timeout_ms: 3000
I (2179728) AUDIO_PROCESSOR: audio_prompt_play_with_block, url: file://spiffs/response_wo_zai_ting_ne.mp3, timeout_ms: -1
I (2179729) ESP_GMF_FILE: Open, dir:1, uri:file://spiffs/response_wo_zai_ting_ne.mp3
I (2179746) ESP_GMF_FILE: File size: 12141 byte, file position: 0
I (2179746) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0131](operator()): Stop audio: file://spiffs/response_wo_zai_ting_ne.mp3
I (2179746) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8194a4,ctx:0x3c69f53c, label:aud_simp_dec_open]
I (2179746) BS:AnimPlayer: [esp_brookesia_anim_player.cpp:0363](processEvent): Update animation: 2, start(0), end(118), fps(30), is_repeat(1)
I (2179748) ESP_GMF_ASMP_DEC: NOTIFY Info, rate: 0, bits: 0, ch: 0 --> rate: 16000, bits: 16, ch: 1
I (2179749) ESP_GMF_TASK: One times job is complete, del[wk:0x3c6b35fc,ctx:0x3c69f618, label:rate_cvt_open]
I (2179749) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8194a4,ctx:0x3c69f704, label:ch_cvt_open]
I (2179750) AUDIO_PROCESSOR: Get info, rate:16000, channels:2, bits:32
I (2179750) AUDIO_PROCESSOR: Get State, 1,ESP_AUD_SIMPLE_PLAYER_RUNNING
I (2179750) ESP_GMF_TASK: One times job is complete, del[wk:0x3c826dbc,ctx:0x3c69f854, label:bit_cvt_open]
I (2179794) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2179795) AUDIO_PROCESSOR: VAD_START
I (2180826) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2180827) AUDIO_PROCESSOR: VAD_END
I (2181134) ESP_GMF_FILE: No more data, ret: 0
I (2181198) ESP_GMF_FILE: No more data, ret: 0
I (2181198) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c8134d0, job:0x3c69f53c-aud_simp_dec_proc]
I (2181198) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c81055c, job:0x3c69f618-rate_cvt_proc]
I (2181198) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c8134f4, job:0x3c69f704-ch_cvt_proc]
I (2181199) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c8106ac, job:0x3c69f854-bit_cvt_proc]
I (2181199) ESP_GMF_FILE: CLose, 0x3c69f9a0, pos = 12141/12141
I (2181200) ESP_GMF_TASK: One times job is complete, del[wk:0x3c81055c,ctx:0x3c69f53c, label:aud_simp_dec_close]
I (2181200) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8106ac,ctx:0x3c69f618, label:rate_cvt_close]
I (2181200) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8194a4,ctx:0x3c69f704, label:ch_cvt_close]
I (2181200) ESP_GMF_TASK: One times job is complete, del[wk:0x3c814fa0,ctx:0x3c69f854, label:bit_cvt_close]
I (2181201) AUDIO_PROCESSOR: Get State, 4,ESP_AUD_SIMPLE_PLAYER_FINISHED
I (2181201) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 1
I (2181266) ESP_COZE_CHAT: speech started
I (2181266) BS:Agent: [coze_chat_app.cpp:0173](audio_event_callback): chat start
I (2181328) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 0
I (2181329) ESP_GMF_TASK: Waiting to run... [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x0, run:0]
I (2181330) ESP_GMF_TASK: Waiting to run... [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x0, run:0]
I (2181512) ESP_COZE_CHAT: speech stopped
I (2181512) BS:Agent: [coze_chat_app.cpp:0176](audio_event_callback): chat stop
I (2182910) BS:Agent: [coze_chat_app.cpp:0123](change_speaking_state): change_speaking_state: 1, force: 0
I (2182911) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0286](operator()): Speaking: true
I (2190354) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2190354) AUDIO_PROCESSOR: VAD_START
I (2192407) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2192407) AUDIO_PROCESSOR: VAD_END
I (2192735) BS:Agent: [coze_chat_app.cpp:0183](audio_event_callback): chat complete
I (2194194) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2194194) AUDIO_PROCESSOR: VAD_START
I (2194737) BS:Agent: [coze_chat_app.cpp:0123](change_speaking_state): change_speaking_state: 0, force: 0
I (2194737) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0286](operator()): Speaking: false
I (2195283) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2195283) AUDIO_PROCESSOR: VAD_END
I (2205275) ESP_COZE_CHAT: speech started
I (2205275) BS:Agent: [coze_chat_app.cpp:0173](audio_event_callback): chat start
I (2205276) ESP_COZE_CHAT: speech stopped
I (2205276) BS:Agent: [coze_chat_app.cpp:0176](audio_event_callback): chat stop
I (2206882) BS:Agent: [coze_chat_app.cpp:0123](change_speaking_state): change_speaking_state: 1, force: 0
I (2206883) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0286](operator()): Speaking: true
I (2209940) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2209941) AUDIO_PROCESSOR: VAD_START
I (2211095) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2211095) AUDIO_PROCESSOR: VAD_END
I (2216136) BS:Agent: [coze_chat_app.cpp:0183](audio_event_callback): chat complete
I (2218140) BS:Agent: [coze_chat_app.cpp:0123](change_speaking_state): change_speaking_state: 0, force: 0
I (2218140) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0286](operator()): Speaking: false
I (2237082) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2237082) AUDIO_PROCESSOR: VAD_START
I (2238549) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2238549) AUDIO_PROCESSOR: VAD_END
I (2248797) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2248798) AUDIO_PROCESSOR: VAD_START
I (2250068) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2250069) AUDIO_PROCESSOR: VAD_END
I (2264221) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2264221) AUDIO_PROCESSOR: VAD_START
I (2284625) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2284625) AUDIO_PROCESSOR: VAD_END
I (2294877) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2294877) AUDIO_PROCESSOR: VAD_START
I (2296283) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2296283) AUDIO_PROCESSOR: VAD_END
I (2297948) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2297948) AUDIO_PROCESSOR: VAD_START
I (2299682) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2299682) AUDIO_PROCESSOR: VAD_END
I (2315288) BS:Agent: [coze_chat_app.cpp:0467](recorder_event_callback_fn): vad start
I (2315288) AUDIO_PROCESSOR: VAD_START
I (2316941) ESP_COZE_CHAT: speech started
I (2316941) BS:Agent: [coze_chat_app.cpp:0173](audio_event_callback): chat start
I (2318363) BS:Agent: [coze_chat_app.cpp:0473](recorder_event_callback_fn): vad end
I (2318363) AUDIO_PROCESSOR: VAD_END
I (2318612) ESP_COZE_CHAT: speech stopped
I (2318613) BS:Agent: [coze_chat_app.cpp:0176](audio_event_callback): chat stop
I (2319863) BS:Agent: [coze_chat_app.cpp:0123](change_speaking_state): change_speaking_state: 1, force: 0
I (2319863) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0286](operator()): Speaking: true
I (2326279) BS:Agent: [coze_chat_app.cpp:0183](audio_event_callback): chat complete
I (2328148) BS:Agent: [coze_chat_app.cpp:0123](change_speaking_state): change_speaking_state: 0, force: 0
I (2328149) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0286](operator()): Speaking: false
I (2328150) Main: [main.cpp:0481](operator()): Switching display mode: UI
I (2328150) ESP_COZE_CHAT: Terminal :{"id":"0ffec44d-3df4-664c-5b79-9fe1c34c03ec","event_type":"conversation.chat.cancel"}
W (2328151) BS:Expression: [esp_brookesia_ai_expression.cpp:0274](setEmotion): Already paused
W (2328152) BS:Expression: [esp_brookesia_ai_expression.cpp:0303](setIcon): Already paused
I (2328151) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0561](processAudioEvent): Play audio: file://spiffs/mic_close.mp3(create_time_ms: 2326123, last_play_time_ms: 0) with timeout_ms: 2000
I (2328157) AUDIO_PROCESSOR: audio_prompt_play_with_block, url: file://spiffs/mic_close.mp3, timeout_ms: -1
I (2328158) ESP_GMF_FILE: Open, dir:1, uri:file://spiffs/mic_close.mp3
I (2328165) ESP_GMF_FILE: File size: 33165 byte, file position: 0
I (2328165) ESP_GMF_TASK: One times job is complete, del[wk:0x3c757648,ctx:0x3c69f53c, label:aud_simp_dec_open]
I (2328167) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0131](operator()): Stop audio: file://spiffs/mic_close.mp3
I (2328168) ESP_GMF_ASMP_DEC: NOTIFY Info, rate: 0, bits: 0, ch: 0 --> rate: 16000, bits: 16, ch: 1
I (2328168) ESP_GMF_TASK: One times job is complete, del[wk:0x3c757648,ctx:0x3c69f618, label:rate_cvt_open]
I (2328169) ESP_GMF_TASK: One times job is complete, del[wk:0x3c818848,ctx:0x3c69f704, label:ch_cvt_open]
I (2328170) AUDIO_PROCESSOR: Get info, rate:16000, channels:2, bits:32
I (2328170) AUDIO_PROCESSOR: Get State, 1,ESP_AUD_SIMPLE_PLAYER_RUNNING
I (2328170) ESP_GMF_TASK: One times job is complete, del[wk:0x3c757648,ctx:0x3c69f854, label:bit_cvt_open]
I (2328258) ESP_COZE_CHAT: Terminal :{"id":"0a100524-d686-55d1-112a-942d437d4224","event_type":"conversation.chat.cancel"}
I (2328363) ESP_COZE_CHAT: Terminal :{"id":"0a39c56c-8ffc-6aec-f4d3-6642bb49f42d","event_type":"conversation.chat.cancel"}
I (2328467) ESP_COZE_CHAT: Terminal :{"id":"1f3fedf7-5c92-0663-962c-fc6e0920ec71","event_type":"conversation.chat.cancel"}
I (2328571) ESP_COZE_CHAT: Terminal :{"id":"08053038-5846-66f8-e4b3-3e153ea9565c","event_type":"conversation.chat.cancel"}
I (2328676) ESP_COZE_CHAT: Terminal :{"id":"1fc397d4-08bb-dcf8-4314-5318bf56b074","event_type":"conversation.chat.cancel"}
I (2328779) ESP_COZE_CHAT: Terminal :{"id":"12ed4f2d-c0ea-2a96-1d2b-6ca20bee991a","event_type":"conversation.chat.cancel"}
I (2328886) ESP_COZE_CHAT: Terminal :{"id":"16d92023-9d82-e2a8-978e-cbfba5e55b0a","event_type":"conversation.chat.cancel"}
I (2328991) ESP_COZE_CHAT: Terminal :{"id":"1a111777-d4a1-6129-356b-6edd97ac8e1a","event_type":"conversation.chat.cancel"}
I (2329095) ESP_COZE_CHAT: Terminal :{"id":"1888751c-e571-f714-75c3-cae3cecd6d39","event_type":"conversation.chat.cancel"}
I (2329200) ESP_COZE_CHAT: Terminal :{"id":"1b809bef-e48f-9c98-2ac0-db08496e20fc","event_type":"conversation.chat.cancel"}
I (2329304) ESP_COZE_CHAT: Terminal :{"id":"078ad77c-4c6b-c717-9f30-97674b3b8c4c","event_type":"conversation.chat.cancel"}
I (2329408) ESP_COZE_CHAT: Terminal :{"id":"101a8a9a-7d0c-de5f-27dd-483ab0270448","event_type":"conversation.chat.cancel"}
I (2329513) ESP_COZE_CHAT: Terminal :{"id":"0001444d-5ef0-76f4-ab27-d724464cf2a3","event_type":"conversation.chat.cancel"}
I (2329617) ESP_COZE_CHAT: Terminal :{"id":"1f21e872-12d1-fd4b-1442-9dbe0d45a2ca","event_type":"conversation.chat.cancel"}
I (2329723) ESP_COZE_CHAT: Terminal :{"id":"11c194f9-a69b-8918-dfd4-2f29afca4bdc","event_type":"conversation.chat.cancel"}
I (2329828) ESP_COZE_CHAT: Terminal :{"id":"12dbf438-de07-7154-9ffa-55966841386d","event_type":"conversation.chat.cancel"}
I (2329932) ESP_COZE_CHAT: Terminal :{"id":"16da54cc-ccb7-2619-bd87-08731a7874ca","event_type":"conversation.chat.cancel"}
I (2330040) ESP_COZE_CHAT: Terminal :{"id":"06288f55-6bb3-aacd-7667-04ac2b1061fa","event_type":"conversation.chat.cancel"}
I (2330144) ESP_COZE_CHAT: Terminal :{"id":"09efa003-a4c0-c8f7-0dba-3e69878ae714","event_type":"conversation.chat.cancel"}
W (2330397) BS:Agent: [coze_chat_app.cpp:0441](recorder_event_callback_fn): chat is not started or paused, skip SAFE event
I (2330398) AUDIO_PROCESSOR: VAD_START
W (2331415) BS:Agent: [coze_chat_app.cpp:0441](recorder_event_callback_fn): chat is not started or paused, skip SAFE event
I (2331415) AUDIO_PROCESSOR: VAD_END
I (2332168) ESP_GMF_FILE: No more data, ret: 0
I (2332243) ESP_GMF_FILE: No more data, ret: 0
I (2332244) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c75766c, job:0x3c69f53c-aud_simp_dec_proc]
I (2332244) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c8199c4, job:0x3c69f618-rate_cvt_proc]
I (2332244) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c81886c, job:0x3c69f704-ch_cvt_proc]
I (2332245) ESP_GMF_TASK: Job is done, [tsk:TSK_0x3c69b2cc-0x3c69b2cc, wk:0x3c8135d0, job:0x3c69f854-bit_cvt_proc]
I (2332245) ESP_GMF_FILE: CLose, 0x3c69f9a0, pos = 33165/33165
I (2332245) ESP_GMF_TASK: One times job is complete, del[wk:0x3c811b88,ctx:0x3c69f53c, label:aud_simp_dec_close]
I (2332246) ESP_GMF_TASK: One times job is complete, del[wk:0x3c764810,ctx:0x3c69f618, label:rate_cvt_close]
I (2332246) ESP_GMF_TASK: One times job is complete, del[wk:0x3c81055c,ctx:0x3c69f704, label:ch_cvt_close]
I (2332246) ESP_GMF_TASK: One times job is complete, del[wk:0x3c8194a4,ctx:0x3c69f854, label:bit_cvt_close]
I (2332246) AUDIO_PROCESSOR: Get State, 4,ESP_AUD_SIMPLE_PLAYER_FINISHED
I (2332247) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 1
I (2332363) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 0