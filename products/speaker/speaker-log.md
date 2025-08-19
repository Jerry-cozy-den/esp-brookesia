分析语音逻辑的log

``` I (24506) AUDIO_PROCESSOR: Get State, 4,ESP_AUD_SIMPLE_PLAYER_FINISHED
I (24507) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 1
I (24642) AUDIO_PROCESSOR: audio_prompt_play_mute, enable_mute: 0
I (24643) ESP_GMF_TASK: Waiting to run... [tsk:TSK_0x3c75549c-0x3c75549c, wk:0x0, run:0]
I (24644) ESP_GMF_TASK: Waiting to run... [tsk:TSK_0x3c75549c-0x3c75549c, wk:0x0, run:0]
I (25158) BS:Agent: [coze_chat_app.cpp:0595](recorder_event_callback_fn): vad end
I (25158) AUDIO_PROCESSOR: VAD_END
--I (25208) ESP_COZE_CHAT: speech started
I (25208) BS:Agent: [coze_chat_app.cpp:0213](audio_event_callback): chat start
I (25209) ESP_COZE_CHAT: speech stopped
I (25209) BS:Agent: [coze_chat_app.cpp:0216](audio_event_callback): chat stop
I (26836) BS:Agent: [coze_chat_app.cpp:0144](change_speaking_state): change_speaking_state: 1, force: 0
I (26836) BS:Speaker: [esp_brookesia_speaker_ai_buddy.cpp:0286](operator()): Speaking: true
I (26839) ESP_GMF_PORT: ACQ OUT, new self payload:0x3c760d74, port:0x3c85c858, el:0x3c85ce9c-aud_simp_dec
W (26840) ESP_GMF_ASMP_DEC: Not enough memory for out, need:1920, old: 1024, new: 1920
I (26847) ESP_GMF_ASMP_DEC: NOTIFY Info, rate: 0, bits: 0, ch: 0 --> rate: 16000, bits: 16, ch: 1
I (26847) ESP_GMF_TASK: One times job is complete, del[wk:0x3c814f70,ctx:0x3c85c788, label:rate_cvt_open]
I (26848) ESP_GMF_TASK: One times job is complete, del[wk:0x3c760b68,ctx:0x3c85c8d8, label:ch_cvt_open]
I (26848) ESP_GMF_PORT: ACQ OUT, new self payload:0x3c760b68, port:0x3c85caf4, el:0x3c85c8d8-ch_cvt
I (26849) AUDIO_PROCESSOR: Get info, rate:16000, channels:2, bits:32
I (26849) AUDIO_PROCESSOR: Get State, 1,ESP_AUD_SIMPLE_PLAYER_RUNNING
I (26849) ESP_GMF_TASK: One times job is complete, del[wk:0x3c814f70,ctx:0x3c85ca28, label:bit_cvt_open]
I (28682) BS:Agent: [coze_chat_app.cpp:0588](recorder_event_callback_fn): vad start
I (28682) AUDIO_PROCESSOR: VAD_START
I (34648) BS:Agent: [coze_chat_app.cpp:0224](audio_event_callback): chat complete
I (36230) BS:Agent: [coze_chat_app.cpp:0595](recorder_event_callback_fn): vad end
I (36230) AUDIO_PROCESSOR: VAD_END