{
  BASS 2.3 Audio Library, (c) 1999-2006 Ian Luck.
  Please report bugs/suggestions/etc... to bass@un4seen.com

  See the BASS.CHM file for more complete documentation


  How to install
  ----------------
  Copy DYNAMIC_BASS.PAS to the \LIB subdirectory of your Delphi path or your project dir

  Call Load_BASSDLL (eg. in FormCreate) to load BASS before using any functions, and
  Unload_BASSDLL (eg. in FormDestory) to unload it when you're done.
}
unit Dynamic_Bass;

interface

uses
  Windows,
  SysUtils;

const
  BASSVERSION = $203;             // API version

  // Use these to test for error from functions that return a DWORD or QWORD
  DW_ERROR = Cardinal(-1); // -1 (DWORD)
  QW_ERROR = Int64(-1);    // -1 (QWORD)

  // Error codes returned by BASS_GetErrorCode()
  BASS_OK                 = 0;    // all is OK
  BASS_ERROR_MEM          = 1;    // memory error
  BASS_ERROR_FILEOPEN     = 2;    // can't open the file
  BASS_ERROR_DRIVER       = 3;    // can't find a free sound driver
  BASS_ERROR_BUFLOST      = 4;    // the sample buffer was lost - please report this!
  BASS_ERROR_HANDLE       = 5;    // invalid handle
  BASS_ERROR_FORMAT       = 6;    // unsupported sample format
  BASS_ERROR_POSITION     = 7;    // invalid playback position
  BASS_ERROR_INIT         = 8;    // BASS_Init has not been successfully called
  BASS_ERROR_START        = 9;    // BASS_Start has not been successfully called
  BASS_ERROR_ALREADY      = 14;   // already initialized/paused/whatever
  BASS_ERROR_NOPAUSE      = 16;   // not paused
  BASS_ERROR_NOCHAN       = 18;   // can't get a free channel
  BASS_ERROR_ILLTYPE      = 19;   // an illegal type was specified
  BASS_ERROR_ILLPARAM     = 20;   // an illegal parameter was specified
  BASS_ERROR_NO3D         = 21;   // no 3D support
  BASS_ERROR_NOEAX        = 22;   // no EAX support
  BASS_ERROR_DEVICE       = 23;   // illegal device number
  BASS_ERROR_NOPLAY       = 24;   // not playing
  BASS_ERROR_FREQ         = 25;   // illegal sample rate
  BASS_ERROR_NOTFILE      = 27;   // the stream is not a file stream
  BASS_ERROR_NOHW         = 29;   // no hardware voices available
  BASS_ERROR_EMPTY        = 31;   // the MOD music has no sequence data
  BASS_ERROR_NONET        = 32;   // no internet connection could be opened
  BASS_ERROR_CREATE       = 33;   // couldn't create the file
  BASS_ERROR_NOFX         = 34;   // effects are not enabled
  BASS_ERROR_PLAYING      = 35;   // the channel is playing
  BASS_ERROR_NOTAVAIL     = 37;   // requested data is not available
  BASS_ERROR_DECODE       = 38;   // the channel is a "decoding channel"
  BASS_ERROR_DX           = 39;   // a sufficient DirectX version is not installed
  BASS_ERROR_TIMEOUT      = 40;   // connection timedout
  BASS_ERROR_FILEFORM     = 41;   // unsupported file format
  BASS_ERROR_SPEAKER      = 42;   // unavailable speaker
  BASS_ERROR_VERSION      = 43;   // invalid BASS version (used by add-ons)
  BASS_ERROR_CODEC        = 44;   // codec is not available/supported
  BASS_ERROR_UNKNOWN      = -1;   // some other mystery error

  // Initialization flags
  BASS_DEVICE_8BITS       = 1;    // use 8 bit resolution, else 16 bit
  BASS_DEVICE_MONO        = 2;    // use mono, else stereo
  BASS_DEVICE_3D          = 4;    // enable 3D functionality
  {
    If the BASS_DEVICE_3D flag is not specified when
    initilizing BASS, then the 3D flags (BASS_SAMPLE_3D
    and BASS_MUSIC_3D) are ignored when loading/creating
    a sample/stream/music.
  }
  BASS_DEVICE_LATENCY     = 256;  // calculate device latency (BASS_INFO struct)
  BASS_DEVICE_SPEAKERS    = 2048; // force enabling of speaker assignment
  BASS_DEVICE_NOSPEAKER   = 4096; // ignore speaker arrangement

  // DirectSound interfaces (for use with BASS_GetDSoundObject)
  BASS_OBJECT_DS          = 1;   // IDirectSound
  BASS_OBJECT_DS3DL       = 2;   // IDirectSound3DListener

  // BASS_INFO flags (from DSOUND.H)
  DSCAPS_CONTINUOUSRATE   = $00000010;
  { supports all sample rates between min/maxrate }
  DSCAPS_EMULDRIVER       = $00000020;
  { device does NOT have hardware DirectSound support }
  DSCAPS_CERTIFIED        = $00000040;
  { device driver has been certified by Microsoft }
  {
    The following flags tell what type of samples are
    supported by HARDWARE mixing, all these formats are
    supported by SOFTWARE mixing
  }
  DSCAPS_SECONDARYMONO    = $00000100;     // mono
  DSCAPS_SECONDARYSTEREO  = $00000200;     // stereo
  DSCAPS_SECONDARY8BIT    = $00000400;     // 8 bit
  DSCAPS_SECONDARY16BIT   = $00000800;     // 16 bit

  // BASS_RECORDINFO flags (from DSOUND.H)
  DSCCAPS_EMULDRIVER = DSCAPS_EMULDRIVER;
  { device does NOT have hardware DirectSound recording support }
  DSCCAPS_CERTIFIED = DSCAPS_CERTIFIED;
  { device driver has been certified by Microsoft }

  // defines for formats field of BASS_RECORDINFO (from MMSYSTEM.H)
  WAVE_FORMAT_1M08       = $00000001;      // 11.025 kHz, Mono,   8-bit
  WAVE_FORMAT_1S08       = $00000002;      // 11.025 kHz, Stereo, 8-bit
  WAVE_FORMAT_1M16       = $00000004;      // 11.025 kHz, Mono,   16-bit
  WAVE_FORMAT_1S16       = $00000008;      // 11.025 kHz, Stereo, 16-bit
  WAVE_FORMAT_2M08       = $00000010;      // 22.05  kHz, Mono,   8-bit
  WAVE_FORMAT_2S08       = $00000020;      // 22.05  kHz, Stereo, 8-bit
  WAVE_FORMAT_2M16       = $00000040;      // 22.05  kHz, Mono,   16-bit
  WAVE_FORMAT_2S16       = $00000080;      // 22.05  kHz, Stereo, 16-bit
  WAVE_FORMAT_4M08       = $00000100;      // 44.1   kHz, Mono,   8-bit
  WAVE_FORMAT_4S08       = $00000200;      // 44.1   kHz, Stereo, 8-bit
  WAVE_FORMAT_4M16       = $00000400;      // 44.1   kHz, Mono,   16-bit
  WAVE_FORMAT_4S16       = $00000800;      // 44.1   kHz, Stereo, 16-bit

  // Sample info flags
  BASS_SAMPLE_8BITS       = 1;   // 8 bit
  BASS_SAMPLE_FLOAT       = 256; // 32-bit floating-point
  BASS_SAMPLE_MONO        = 2;   // mono, else stereo
  BASS_SAMPLE_LOOP        = 4;   // looped
  BASS_SAMPLE_3D          = 8;   // 3D functionality enabled
  BASS_SAMPLE_SOFTWARE    = 16;  // it's NOT using hardware mixing
  BASS_SAMPLE_MUTEMAX     = 32;  // muted at max distance (3D only)
  BASS_SAMPLE_VAM         = 64;  // uses the DX7 voice allocation & management
  BASS_SAMPLE_FX          = 128; // old implementation of DX8 effects are enabled
  BASS_SAMPLE_OVER_VOL    = $10000; // override lowest volume
  BASS_SAMPLE_OVER_POS    = $20000; // override longest playing
  BASS_SAMPLE_OVER_DIST   = $30000; // override furthest from listener (3D only)

  BASS_STREAM_PRESCAN     = $20000; // enable pin-point seeking (MP3/MP2/MP1)
  BASS_MP3_SETPOS         = BASS_STREAM_PRESCAN;
  BASS_STREAM_AUTOFREE	  = $40000; // automatically free the stream when it stop/ends
  BASS_STREAM_RESTRATE	  = $80000; // restrict the download rate of internet file streams
  BASS_STREAM_BLOCK       = $100000;// download/play internet file stream in small blocks
  BASS_STREAM_DECODE      = $200000;// don't play the stream, only decode (BASS_ChannelGetData)
  BASS_STREAM_STATUS      = $800000;// give server status info (HTTP/ICY tags) in DOWNLOADPROC

  BASS_MUSIC_FLOAT        = BASS_SAMPLE_FLOAT; // 32-bit floating-point
  BASS_MUSIC_MONO         = BASS_SAMPLE_MONO; // force mono mixing (less CPU usage)
  BASS_MUSIC_LOOP         = BASS_SAMPLE_LOOP; // loop music
  BASS_MUSIC_3D           = BASS_SAMPLE_3D; // enable 3D functionality
  BASS_MUSIC_FX           = BASS_SAMPLE_FX; // enable old implementation of DX8 effects
  BASS_MUSIC_AUTOFREE     = BASS_STREAM_AUTOFREE; // automatically free the music when it stop/ends
  BASS_MUSIC_DECODE       = BASS_STREAM_DECODE; // don't play the music, only decode (BASS_ChannelGetData)
  BASS_MUSIC_PRESCAN      = BASS_STREAM_PRESCAN; // calculate playback length
  BASS_MUSIC_CALCLEN      = BASS_MUSIC_PRESCAN;
  BASS_MUSIC_RAMP         = $200;  // normal ramping
  BASS_MUSIC_RAMPS        = $400;  // sensitive ramping
  BASS_MUSIC_SURROUND     = $800;  // surround sound
  BASS_MUSIC_SURROUND2    = $1000; // surround sound (mode 2)
  BASS_MUSIC_FT2MOD       = $2000; // play .MOD as FastTracker 2 does
  BASS_MUSIC_PT1MOD       = $4000; // play .MOD as ProTracker 1 does
  BASS_MUSIC_NONINTER     = $10000; // non-interpolated mixing
  BASS_MUSIC_POSRESET     = $8000; // stop all notes when moving position
  BASS_MUSIC_POSRESETEX   = $400000; // stop all notes and reset bmp/etc when moving position
  BASS_MUSIC_STOPBACK     = $80000; // stop the music on a backwards jump effect
  BASS_MUSIC_NOSAMPLE     = $100000; // don't load the samples

  // Speaker assignment flags
  BASS_SPEAKER_FRONT      = $1000000;  // front speakers
  BASS_SPEAKER_REAR       = $2000000;  // rear/side speakers
  BASS_SPEAKER_CENLFE     = $3000000;  // center & LFE speakers (5.1)
  BASS_SPEAKER_REAR2      = $4000000;  // rear center speakers (7.1)
  BASS_SPEAKER_LEFT       = $10000000; // modifier: left
  BASS_SPEAKER_RIGHT      = $20000000; // modifier: right
  BASS_SPEAKER_FRONTLEFT  = BASS_SPEAKER_FRONT or BASS_SPEAKER_LEFT;
  BASS_SPEAKER_FRONTRIGHT = BASS_SPEAKER_FRONT or BASS_SPEAKER_RIGHT;
  BASS_SPEAKER_REARLEFT   = BASS_SPEAKER_REAR or BASS_SPEAKER_LEFT;
  BASS_SPEAKER_REARRIGHT  = BASS_SPEAKER_REAR or BASS_SPEAKER_RIGHT;
  BASS_SPEAKER_CENTER     = BASS_SPEAKER_CENLFE or BASS_SPEAKER_LEFT;
  BASS_SPEAKER_LFE        = BASS_SPEAKER_CENLFE or BASS_SPEAKER_RIGHT;
  BASS_SPEAKER_REAR2LEFT  = BASS_SPEAKER_REAR2 or BASS_SPEAKER_LEFT;
  BASS_SPEAKER_REAR2RIGHT = BASS_SPEAKER_REAR2 or BASS_SPEAKER_RIGHT;

  BASS_UNICODE            = $80000000;

  BASS_RECORD_PAUSE       = $8000; // start recording paused

  // DX7 voice allocation flags
  BASS_VAM_HARDWARE       = 1;
  {
    Play the sample in hardware. If no hardware voices are available then
    the "play" call will fail
  }
  BASS_VAM_SOFTWARE       = 2;
  {
    Play the sample in software (ie. non-accelerated). No other VAM flags
    may be used together with this flag.
  }

  // DX7 voice management flags
  {
    These flags enable hardware resource stealing... if the hardware has no
    available voices, a currently playing buffer will be stopped to make room
    for the new buffer. NOTE: only samples loaded/created with the
    BASS_SAMPLE_VAM flag are considered for termination by the DX7 voice
    management.
  }
  BASS_VAM_TERM_TIME      = 4;
  {
    If there are no free hardware voices, the buffer to be terminated will be
    the one with the least time left to play.
  }
  BASS_VAM_TERM_DIST      = 8;
  {
    If there are no free hardware voices, the buffer to be terminated will be
    one that was loaded/created with the BASS_SAMPLE_MUTEMAX flag and is
    beyond
    it's max distance. If there are no buffers that match this criteria, then
    the "play" call will fail.
  }
  BASS_VAM_TERM_PRIO      = 16;
  {
    If there are no free hardware voices, the buffer to be terminated will be
    the one with the lowest priority.
  }

  // BASS_CHANNELINFO types
  BASS_CTYPE_SAMPLE       = 1;
  BASS_CTYPE_RECORD       = 2;
  BASS_CTYPE_STREAM       = $10000;
  BASS_CTYPE_STREAM_OGG   = $10002;
  BASS_CTYPE_STREAM_MP1   = $10003;
  BASS_CTYPE_STREAM_MP2   = $10004;
  BASS_CTYPE_STREAM_MP3   = $10005;
  BASS_CTYPE_STREAM_AIFF  = $10006;
  BASS_CTYPE_STREAM_WAV   = $40000; // WAVE flag, LOWORD=codec
  BASS_CTYPE_STREAM_WAV_PCM = $50001;
  BASS_CTYPE_STREAM_WAV_FLOAT = $50003;
  BASS_CTYPE_MUSIC_MOD    = $20000;
  BASS_CTYPE_MUSIC_MTM    = $20001;
  BASS_CTYPE_MUSIC_S3M    = $20002;
  BASS_CTYPE_MUSIC_XM     = $20003;
  BASS_CTYPE_MUSIC_IT     = $20004;
  BASS_CTYPE_MUSIC_MO3    = $00100; // MO3 flag

  // 3D channel modes
  BASS_3DMODE_NORMAL      = 0;
  { normal 3D processing }
  BASS_3DMODE_RELATIVE    = 1;
  {
    The channel's 3D position (position/velocity/
    orientation) are relative to the listener. When the
    listener's position/velocity/orientation is changed
    with BASS_Set3DPosition, the channel's position
    relative to the listener does not change.
  }
  BASS_3DMODE_OFF         = 2;
  {
    Turn off 3D processing on the channel, the sound will
    be played in the center.
  }

  // EAX environments, use with BASS_SetEAXParameters
  EAX_ENVIRONMENT_OFF               = -1;
  EAX_ENVIRONMENT_GENERIC           = 0;
  EAX_ENVIRONMENT_PADDEDCELL        = 1;
  EAX_ENVIRONMENT_ROOM              = 2;
  EAX_ENVIRONMENT_BATHROOM          = 3;
  EAX_ENVIRONMENT_LIVINGROOM        = 4;
  EAX_ENVIRONMENT_STONEROOM         = 5;
  EAX_ENVIRONMENT_AUDITORIUM        = 6;
  EAX_ENVIRONMENT_CONCERTHALL       = 7;
  EAX_ENVIRONMENT_CAVE              = 8;
  EAX_ENVIRONMENT_ARENA             = 9;
  EAX_ENVIRONMENT_HANGAR            = 10;
  EAX_ENVIRONMENT_CARPETEDHALLWAY   = 11;
  EAX_ENVIRONMENT_HALLWAY           = 12;
  EAX_ENVIRONMENT_STONECORRIDOR     = 13;
  EAX_ENVIRONMENT_ALLEY             = 14;
  EAX_ENVIRONMENT_FOREST            = 15;
  EAX_ENVIRONMENT_CITY              = 16;
  EAX_ENVIRONMENT_MOUNTAINS         = 17;
  EAX_ENVIRONMENT_QUARRY            = 18;
  EAX_ENVIRONMENT_PLAIN             = 19;
  EAX_ENVIRONMENT_PARKINGLOT        = 20;
  EAX_ENVIRONMENT_SEWERPIPE         = 21;
  EAX_ENVIRONMENT_UNDERWATER        = 22;
  EAX_ENVIRONMENT_DRUGGED           = 23;
  EAX_ENVIRONMENT_DIZZY             = 24;
  EAX_ENVIRONMENT_PSYCHOTIC         = 25;
  // total number of environments
  EAX_ENVIRONMENT_COUNT             = 26;

  // software 3D mixing algorithm modes (used with BASS_Set3DAlgorithm)
  BASS_3DALG_DEFAULT                = 0;
  {
    default algorithm (currently translates to BASS_3DALG_OFF)
  }
  BASS_3DALG_OFF                    = 1;
  {
    Uses normal left and right panning. The vertical axis is ignored except
    for scaling of volume due to distance. Doppler shift and volume scaling
    are still applied, but the 3D filtering is not performed. This is the
    most CPU efficient software implementation, but provides no virtual 3D
    audio effect. Head Related Transfer Function processing will not be done.
    Since only normal stereo panning is used, a channel using this algorithm
    may be accelerated by a 2D hardware voice if no free 3D hardware voices
    are available.
  }
  BASS_3DALG_FULL                   = 2;
  {
    This algorithm gives the highest quality 3D audio effect, but uses more
    CPU. Requires Windows 98 2nd Edition or Windows 2000 that uses WDM
    drivers, if this mode is not available then BASS_3DALG_OFF will be used
    instead.
  }
  BASS_3DALG_LIGHT                  = 3;
  {
    This algorithm gives a good 3D audio effect, and uses less CPU than the
    FULL mode. Requires Windows 98 2nd Edition or Windows 2000 that uses WDM
    drivers, if this mode is not available then BASS_3DALG_OFF will be used
    instead.
  }

  {
    Sync types (with BASS_ChannelSetSync() "param" and
    SYNCPROC "data" definitions) & flags.
  }
  BASS_SYNC_POS                     = 0;
  {
    Sync when a channel reaches a position.
    param: position in bytes
    data : not used
  }
  BASS_SYNC_END                     = 2;
  {
    Sync when a channel reaches the end.
    param: not used
    data : not used
  }
  BASS_SYNC_META                    = 4;
  {
    Sync when metadata is received in a stream.
    param: not used
    data : pointer to the metadata
  }
  BASS_SYNC_SLIDE                   = 5;
  {
    Sync when an attribute slide is completed.
    param: not used
    data : the type of slide completed (one of the BASS_SLIDE_xxx values)
  }
  BASS_SYNC_STALL                   = 6;
  {
    Sync when playback has stalled.
    param: not used
    data : 0=stalled, 1=resumed
  }
  BASS_SYNC_DOWNLOAD                = 7;
  {
    Sync when downloading of an internet (or "buffered" user file) stream has ended.
    param: not used
    data : not used
  }
  BASS_SYNC_FREE                    = 8;
  {
    Sync when a channel is freed.
    param: not used
    data : not used
  }
  BASS_SYNC_MUSICPOS                = 10;
  {
    Sync when a MOD music reaches an order:row position.
    param: LOWORD=order (0=first, -1=all) HIWORD=row (0=first, -1=all)
    data : LOWORD=order HIWORD=row
  }
  BASS_SYNC_MUSICINST               = 1;
  {
    Sync when an instrument (sample for the non-instrument based formats)
    is played in a MOD music (not including retrigs).
    param: LOWORD=instrument (1=first) HIWORD=note (0=c0...119=b9, -1=all)
    data : LOWORD=note HIWORD=volume (0-64)
  }
  BASS_SYNC_MUSICFX                 = 3;
  {
    Sync when the "sync" effect (XM/MTM/MOD: E8x/Wxx, IT/S3M: S2x) is used.
    param: 0:data=pos, 1:data="x" value
    data : param=0: LOWORD=order HIWORD=row, param=1: "x" value
  }
  BASS_SYNC_MESSAGE                 = $20000000;
  { FLAG: post a Windows message (instead of callback)
    When using a window message "callback", the message to post is given in the "proc"
    parameter of BASS_ChannelSetSync, and is posted to the window specified in the BASS_Init
    call. The message parameters are: WPARAM = data, LPARAM = user.
  }
  BASS_SYNC_MIXTIME                 = $40000000;
  { FLAG: sync at mixtime, else at playtime }
  BASS_SYNC_ONETIME                 = $80000000;
  { FLAG: sync only once, else continuously }

  // BASS_ChannelIsActive return values
  BASS_ACTIVE_STOPPED = 0;
  BASS_ACTIVE_PLAYING = 1;
  BASS_ACTIVE_STALLED = 2;
  BASS_ACTIVE_PAUSED  = 3;

  // BASS_ChannelIsSliding return flags
  BASS_SLIDE_FREQ     = 1;
  BASS_SLIDE_VOL      = 2;
  BASS_SLIDE_PAN      = 4;

  // BASS_ChannelGetData flags
  BASS_DATA_AVAILABLE = 0;        // query how much data is buffered
  BASS_DATA_FLOAT     = $40000000; // flag: return floating-point sample data
  BASS_DATA_FFT512    = $80000000; // 512 sample FFT
  BASS_DATA_FFT1024   = $80000001; // 1024 FFT
  BASS_DATA_FFT2048   = $80000002; // 2048 FFT
  BASS_DATA_FFT4096   = $80000003; // 4096 FFT
  BASS_DATA_FFT_INDIVIDUAL = $10; // FFT flag: FFT for each channel, else all combined
  BASS_DATA_FFT_NOWINDOW = $20;   // FFT flag: no Hanning window

  // BASS_ChannelGetTags types : what's returned
  BASS_TAG_ID3        = 0; // ID3v1 tags : 128 byte block
  BASS_TAG_ID3V2      = 1; // ID3v2 tags : variable length block
  BASS_TAG_OGG        = 2; // OGG comments : array of null-terminated strings
  BASS_TAG_HTTP       = 3; // HTTP headers : array of null-terminated strings
  BASS_TAG_ICY        = 4; // ICY headers : array of null-terminated strings
  BASS_TAG_META       = 5; // ICY metadata : null-terminated string
  BASS_TAG_VENDOR     = 9; // OGG encoder : null-terminated string
  BASS_TAG_RIFF_INFO  = $100; // RIFF/WAVE tags : array of null-terminated ANSI strings
  BASS_TAG_MUSIC_NAME = $10000;	// MOD music name : ANSI string
  BASS_TAG_MUSIC_MESSAGE = $10001; // MOD message : ANSI string
  BASS_TAG_MUSIC_INST = $10100;	// + instrument #, MOD instrument name : ANSI string
  BASS_TAG_MUSIC_SAMPLE = $10300; // + sample #, MOD sample name : ANSI string

  BASS_FX_CHORUS	  = 0;      // GUID_DSFX_STANDARD_CHORUS
  BASS_FX_COMPRESSOR  = 1;      // GUID_DSFX_STANDARD_COMPRESSOR
  BASS_FX_DISTORTION  = 2;      // GUID_DSFX_STANDARD_DISTORTION
  BASS_FX_ECHO        = 3;      // GUID_DSFX_STANDARD_ECHO
  BASS_FX_FLANGER     = 4;      // GUID_DSFX_STANDARD_FLANGER
  BASS_FX_GARGLE      = 5;      // GUID_DSFX_STANDARD_GARGLE
  BASS_FX_I3DL2REVERB = 6;      // GUID_DSFX_STANDARD_I3DL2REVERB
  BASS_FX_PARAMEQ     = 7;      // GUID_DSFX_STANDARD_PARAMEQ
  BASS_FX_REVERB      = 8;      // GUID_DSFX_WAVES_REVERB

  BASS_FX_PHASE_NEG_180 = 0;
  BASS_FX_PHASE_NEG_90  = 1;
  BASS_FX_PHASE_ZERO    = 2;
  BASS_FX_PHASE_90      = 3;
  BASS_FX_PHASE_180     = 4;

  // BASS_RecordSetInput flags
  BASS_INPUT_OFF    = $10000;
  BASS_INPUT_ON     = $20000;
  BASS_INPUT_LEVEL  = $40000;

  BASS_INPUT_TYPE_MASK    = $ff000000;
  BASS_INPUT_TYPE_UNDEF   = $00000000;
  BASS_INPUT_TYPE_DIGITAL = $01000000;
  BASS_INPUT_TYPE_LINE    = $02000000;
  BASS_INPUT_TYPE_MIC     = $03000000;
  BASS_INPUT_TYPE_SYNTH   = $04000000;
  BASS_INPUT_TYPE_CD      = $05000000;
  BASS_INPUT_TYPE_PHONE   = $06000000;
  BASS_INPUT_TYPE_SPEAKER = $07000000;
  BASS_INPUT_TYPE_WAVE    = $08000000;
  BASS_INPUT_TYPE_AUX     = $09000000;
  BASS_INPUT_TYPE_ANALOG  = $0a000000;

  // BASS_SetNetConfig flags
  BASS_NET_TIMEOUT  = 0;
  BASS_NET_BUFFER   = 1;

  // BASS_StreamGetFilePosition modes
  BASS_FILEPOS_CURRENT    = 0;
  BASS_FILEPOS_DECODE     = BASS_FILEPOS_CURRENT;
  BASS_FILEPOS_DOWNLOAD   = 1;
  BASS_FILEPOS_END        = 2;
  BASS_FILEPOS_START      = 3;

  // STREAMFILEPROC actions
  BASS_FILE_CLOSE   = 0;
  BASS_FILE_READ    = 1;
  BASS_FILE_LEN     = 3;
  BASS_FILE_SEEK    = 4;

  BASS_STREAMPROC_END = $80000000; // end of user stream flag

  // BASS_MusicSet/GetAttribute options
  BASS_MUSIC_ATTRIB_AMPLIFY    = 0;
  BASS_MUSIC_ATTRIB_PANSEP     = 1;
  BASS_MUSIC_ATTRIB_PSCALER    = 2;
  BASS_MUSIC_ATTRIB_BPM        = 3;
  BASS_MUSIC_ATTRIB_SPEED      = 4;
  BASS_MUSIC_ATTRIB_VOL_GLOBAL = 5;
  BASS_MUSIC_ATTRIB_VOL_CHAN   = $100; // + channel #
  BASS_MUSIC_ATTRIB_VOL_INST   = $200; // + instrument #

  // BASS_Set/GetConfig options
  BASS_CONFIG_BUFFER        = 0;
  BASS_CONFIG_UPDATEPERIOD  = 1;
  BASS_CONFIG_MAXVOL        = 3;
  BASS_CONFIG_GVOL_SAMPLE   = 4;
  BASS_CONFIG_GVOL_STREAM   = 5;
  BASS_CONFIG_GVOL_MUSIC    = 6;
  BASS_CONFIG_CURVE_VOL     = 7;
  BASS_CONFIG_CURVE_PAN     = 8;
  BASS_CONFIG_FLOATDSP      = 9;
  BASS_CONFIG_3DALGORITHM   = 10;
  BASS_CONFIG_NET_TIMEOUT   = 11;
  BASS_CONFIG_NET_BUFFER    = 12;
  BASS_CONFIG_PAUSE_NOPLAY  = 13;
  BASS_CONFIG_NET_PREBUF    = 15;
  BASS_CONFIG_NET_AGENT     = 16;
  BASS_CONFIG_NET_PROXY     = 17;
  BASS_CONFIG_NET_PASSIVE   = 18;
  BASS_CONFIG_MP3_CODEC     = 19;

type
  DWORD = cardinal;
  BOOL = LongBool;
  FLOAT = Single;
  QWORD = int64;        // 64-bit (replace "int64" with "comp" if using Delphi 3)

  HMUSIC = DWORD;       // MOD music handle
  HSAMPLE = DWORD;      // sample handle
  HCHANNEL = DWORD;     // playing sample's channel handle
  HSTREAM = DWORD;      // sample stream handle
  HRECORD = DWORD;      // recording handle
  HSYNC = DWORD;        // synchronizer handle
  HDSP = DWORD;         // DSP handle
  HFX = DWORD;          // DX8 effect handle
  HPLUGIN = DWORD;      // Plugin handle

  BASS_INFO = record
    flags: DWORD;       // device capabilities (DSCAPS_xxx flags)
    hwsize: DWORD;      // size of total device hardware memory
    hwfree: DWORD;      // size of free device hardware memory
    freesam: DWORD;     // number of free sample slots in the hardware
    free3d: DWORD;      // number of free 3D sample slots in the hardware
    minrate: DWORD;     // min sample rate supported by the hardware
    maxrate: DWORD;     // max sample rate supported by the hardware
    eax: BOOL;          // device supports EAX? (always FALSE if BASS_DEVICE_3D was not used)
    minbuf: DWORD;      // recommended minimum buffer length in ms (requires BASS_DEVICE_LATENCY)
    dsver: DWORD;       // DirectSound version
    latency: DWORD;     // delay (in ms) before start of playback (requires BASS_DEVICE_LATENCY)
    initflags: DWORD;   // "flags" parameter of BASS_Init call
    speakers: DWORD;    // number of speakers available
	driver: PChar;      // driver
    freq: DWORD;        // current output rate (OSX only)
  end;

  BASS_RECORDINFO = record
    flags: DWORD;       // device capabilities (DSCCAPS_xxx flags)
    formats: DWORD;     // supported standard formats (WAVE_FORMAT_xxx flags)
    inputs: DWORD;      // number of inputs
    singlein: BOOL;     // only 1 input can be set at a time
	driver: PChar;      // driver
    freq: DWORD;        // current input rate (OSX only)
  end;

  BASS_CHANNELINFO = record
	freq: DWORD;        // default playback rate
	chans: DWORD;       // channels
	flags: DWORD;       // BASS_SAMPLE/STREAM/MUSIC/SPEAKER flags
	ctype: DWORD;       // type of channel
	origres: DWORD;     // original resolution
	plugin: HPLUGIN;    // plugin
  end;

  BASS_PLUGINFORM = record
	ctype: DWORD;       // channel type
	name: PChar;        // format description
	exts: PChar;	    // file extension filter (*.ext1;*.ext2;etc...)
  end;
  PBASS_PLUGINFORMS = ^TBASS_PLUGINFORMS;
  TBASS_PLUGINFORMS = array[0..maxInt div sizeOf(BASS_PLUGINFORM) - 1] of BASS_PLUGINFORM;

  BASS_PLUGININFO = record
	version: DWORD;             // version (same form as BASS_GetVersion)
	formatc: DWORD;             // number of formats
	formats: PBASS_PLUGINFORMS; // the array of formats
  end;
  PBASS_PLUGININFO = ^BASS_PLUGININFO;

  // Sample info structure
  BASS_SAMPLE = record
    freq: DWORD;        // default playback rate
    volume: DWORD;      // default volume (0-100)
    pan: Integer;       // default pan (-100=left, 0=middle, 100=right)
    flags: DWORD;       // BASS_SAMPLE_xxx flags
    length: DWORD;      // length (in samples, not bytes)
    max: DWORD;         // maximum simultaneous playbacks
    origres: DWORD;     // original resolution
    chans: DWORD;       // number of channels
    mingap: DWORD;      // minimum gap (ms) between creating channels
    {
      The following are the sample's default 3D attributes
      (if the sample is 3D, BASS_SAMPLE_3D is in flags)
      see BASS_ChannelSet3DAttributes
    }
    mode3d: DWORD;      // BASS_3DMODE_xxx mode
    mindist: FLOAT;     // minimum distance
    maxdist: FLOAT;     // maximum distance
    iangle: DWORD;      // angle of inside projection cone
    oangle: DWORD;      // angle of outside projection cone
    outvol: DWORD;      // delta-volume outside the projection cone
    {
      The following are the defaults used if the sample uses the DirectX 7
      voice allocation/management features.
    }
    vam: DWORD;         // voice allocation/management flags (BASS_VAM_xxx)
    priority: DWORD;    // priority (0=lowest, $ffffffff=highest)
  end;

  // 3D vector (for 3D positions/velocities/orientations)
  BASS_3DVECTOR = record
    x: FLOAT;           // +=right, -=left
    y: FLOAT;           // +=up, -=down
    z: FLOAT;           // +=front, -=behind
  end;

  BASS_FXCHORUS = record
    fWetDryMix: FLOAT;
    fDepth: FLOAT;
    fFeedback: FLOAT;
    fFrequency: FLOAT;
    lWaveform: DWORD;   // 0=triangle, 1=sine
    fDelay: FLOAT;
    lPhase: DWORD;      // BASS_FX_PHASE_xxx
  end;

  BASS_FXCOMPRESSOR = record
    fGain: FLOAT;
    fAttack: FLOAT;
    fRelease: FLOAT;
    fThreshold: FLOAT;
    fRatio: FLOAT;
    fPredelay: FLOAT;
  end;

  BASS_FXDISTORTION = record
    fGain: FLOAT;
    fEdge: FLOAT;
    fPostEQCenterFrequency: FLOAT;
    fPostEQBandwidth: FLOAT;
    fPreLowpassCutoff: FLOAT;
  end;

  BASS_FXECHO = record
    fWetDryMix: FLOAT;
    fFeedback: FLOAT;
    fLeftDelay: FLOAT;
    fRightDelay: FLOAT;
    lPanDelay: BOOL;
  end;

  BASS_FXFLANGER = record
    fWetDryMix: FLOAT;
    fDepth: FLOAT;
    fFeedback: FLOAT;
    fFrequency: FLOAT;
    lWaveform: DWORD;   // 0=triangle, 1=sine
    fDelay: FLOAT;
    lPhase: DWORD;      // BASS_FX_PHASE_xxx
  end;

  BASS_FXGARGLE = record
    dwRateHz: DWORD;               // Rate of modulation in hz
    dwWaveShape: DWORD;            // 0=triangle, 1=square
  end;

  BASS_FXI3DL2REVERB = record
    lRoom: Longint;                // [-10000, 0]      default: -1000 mB
    lRoomHF: Longint;              // [-10000, 0]      default: 0 mB
    flRoomRolloffFactor: FLOAT;    // [0.0, 10.0]      default: 0.0
    flDecayTime: FLOAT;            // [0.1, 20.0]      default: 1.49s
    flDecayHFRatio: FLOAT;         // [0.1, 2.0]       default: 0.83
    lReflections: Longint;         // [-10000, 1000]   default: -2602 mB
    flReflectionsDelay: FLOAT;     // [0.0, 0.3]       default: 0.007 s
    lReverb: Longint;              // [-10000, 2000]   default: 200 mB
    flReverbDelay: FLOAT;          // [0.0, 0.1]       default: 0.011 s
    flDiffusion: FLOAT;            // [0.0, 100.0]     default: 100.0 %
    flDensity: FLOAT;              // [0.0, 100.0]     default: 100.0 %
    flHFReference: FLOAT;          // [20.0, 20000.0]  default: 5000.0 Hz
  end;

  BASS_FXPARAMEQ = record
    fCenter: FLOAT;
    fBandwidth: FLOAT;
    fGain: FLOAT;
  end;

  BASS_FXREVERB = record
    fInGain: FLOAT;                // [-96.0,0.0]            default: 0.0 dB
    fReverbMix: FLOAT;             // [-96.0,0.0]            default: 0.0 db
    fReverbTime: FLOAT;            // [0.001,3000.0]         default: 1000.0 ms
    fHighFreqRTRatio: FLOAT;       // [0.001,0.999]          default: 0.001
  end;

  // callback function types
  STREAMPROC = function(handle: HSTREAM; buffer: Pointer; length: DWORD; user: DWORD): DWORD; stdcall;
  {
    User stream callback function. NOTE: A stream function should obviously be as
    quick as possible, other streams (and MOD musics) can't be mixed until
    it's finished.
    handle : The stream that needs writing
    buffer : Buffer to write the samples in
    length : Number of bytes to write
    user   : The 'user' parameter value given when calling BASS_StreamCreate
    RETURN : Number of bytes written. Set the BASS_STREAMPROC_END flag to end
             the stream.
  }

  STREAMFILEPROC = function(action, param1, param2, user: DWORD): DWORD; stdcall;
  {  
    User file stream callback function.
    action : The action to perform, one of BASS_FILE_xxx values.
    param1 : Depends on "action"
    param2 : Depends on "action"
    user   : The 'user' parameter value given when calling BASS_StreamCreate
    RETURN : Depends on "action"
  }

  DOWNLOADPROC = procedure(buffer: Pointer; length: DWORD; user: DWORD); stdcall;
  {
    Internet stream download callback function.
    buffer : Buffer containing the downloaded data... NULL=end of download
    length : Number of bytes in the buffer
    user   : The 'user' parameter value given when calling BASS_StreamCreateURL
  }

  SYNCPROC = procedure(handle: HSYNC; channel, data: DWORD; user: DWORD); stdcall;
  {
    Sync callback function. NOTE: a sync callback function should be very
    quick as other syncs cannot be processed until it has finished. If the
    sync is a "mixtime" sync, then other streams and MOD musics can not be
    mixed until it's finished either.
    handle : The sync that has occured
    channel: Channel that the sync occured in
    data   : Additional data associated with the sync's occurance
    user   : The 'user' parameter given when calling BASS_ChannelSetSync
  }

  DSPPROC = procedure(handle: HDSP; channel: DWORD; buffer: Pointer; length: DWORD; user: DWORD); stdcall;
  {
    DSP callback function. NOTE: A DSP function should obviously be as quick
    as possible... other DSP functions, streams and MOD musics can not be
    processed until it's finished.
    handle : The DSP handle
    channel: Channel that the DSP is being applied to
    buffer : Buffer to apply the DSP to
    length : Number of bytes in the buffer
    user   : The 'user' parameter given when calling BASS_ChannelSetDSP
  }

  RECORDPROC = function(handle: HRECORD; buffer: Pointer; length: DWORD; user: DWORD): BOOL; stdcall;
  {
    Recording callback function.
    handle : The recording handle
    buffer : Buffer containing the recorded sample data
    length : Number of bytes
    user   : The 'user' parameter value given when calling BASS_RecordStart
    RETURN : TRUE = continue recording, FALSE = stop
  }


// Vars that will hold our dynamically loaded functions...hopefully I didn't miss one of them...if I did miss one (stupid, I know !-; ) please let me know which one it was !
var BASS_SetConfig:function(option, value: DWORD): DWORD; stdcall;
var BASS_GetConfig:function(option: DWORD): DWORD; stdcall;
var BASS_GetVersion:function: DWORD; stdcall;
var BASS_GetDeviceDescription:function(device: DWORD): PChar; stdcall;
var BASS_ErrorGetCode:function: DWORD; stdcall;
var BASS_Init:function(device: Integer; freq, flags: DWORD; win: HWND; clsid: PGUID): BOOL; stdcall;
var BASS_SetDevice:function(device: DWORD): BOOL; stdcall;
var BASS_GetDevice:function: DWORD; stdcall;
var BASS_Free:function: BOOL; stdcall;
var BASS_GetDSoundObject:function(obj: DWORD): Pointer; stdcall;
var BASS_GetInfo:function(var info: BASS_INFO): BOOL; stdcall;
var BASS_Update:function: BOOL; stdcall;
var BASS_GetCPU:function: FLOAT; stdcall;
var BASS_Start:function: BOOL; stdcall;
var BASS_Stop:function: BOOL; stdcall;
var BASS_Pause:function: BOOL; stdcall;
var BASS_SetVolume:function(volume: DWORD): BOOL; stdcall;
var BASS_GetVolume:function: Integer; stdcall;

var BASS_PluginLoad:function(f: PChar; flags: DWORD): HPLUGIN; stdcall;
var BASS_PluginFree:function(handle: HPLUGIN): BOOL; stdcall;
var BASS_PluginGetInfo:function(handle: HPLUGIN): PBASS_PLUGININFO; stdcall;

var BASS_Set3DFactors:function(distf, rollf, doppf: FLOAT): BOOL; stdcall;
var BASS_Get3DFactors:function(var distf, rollf, doppf: FLOAT): BOOL; stdcall;
var BASS_Set3DPosition:function(var pos, vel, front, top: BASS_3DVECTOR): BOOL; stdcall;
var BASS_Get3DPosition:function(var pos, vel, front, top: BASS_3DVECTOR): BOOL; stdcall;
var BASS_Apply3D:procedure; stdcall;
var BASS_SetEAXParameters:function(env: Integer; vol, decay, damp: FLOAT): BOOL; stdcall;
var BASS_GetEAXParameters:function(var env: DWORD; var vol, decay, damp: FLOAT): BOOL; stdcall;

var BASS_MusicLoad:function(mem: BOOL; f: Pointer; offset, length, flags, freq: DWORD): HMUSIC; stdcall;
var BASS_MusicFree:function(handle: HMUSIC): BOOL; stdcall;
var BASS_MusicSetAttribute:function(handle: HMUSIC; attrib,value: DWORD): DWORD; stdcall;
var BASS_MusicGetAttribute:function(handle: HMUSIC; attrib: DWORD): DWORD; stdcall;
var BASS_MusicGetOrders:function(handle: HMUSIC): DWORD; stdcall;
var BASS_MusicGetOrderPosition:function(handle: HMUSIC): DWORD; stdcall;

var BASS_SampleLoad:function(mem: BOOL; f: Pointer; offset, length, max, flags: DWORD): HSAMPLE; stdcall;
var BASS_SampleCreate:function(length, freq, chans, max, flags: DWORD): Pointer; stdcall;
var BASS_SampleCreateDone:function: HSAMPLE; stdcall;
var BASS_SampleFree:function(handle: HSAMPLE): BOOL; stdcall;
var BASS_SampleGetInfo:function(handle: HSAMPLE; var info: BASS_SAMPLE): BOOL; stdcall;
var BASS_SampleSetInfo:function(handle: HSAMPLE; var info: BASS_SAMPLE): BOOL; stdcall;
var BASS_SampleGetChannel:function(handle: HSAMPLE; onlynew: BOOL): HCHANNEL; stdcall;
var BASS_SampleStop:function(handle: HSAMPLE): BOOL; stdcall;

var BASS_StreamCreate:function(freq, chans, flags: DWORD; proc: Pointer; user: DWORD): HSTREAM; stdcall;
var BASS_StreamCreateFile:function(mem: BOOL; f: Pointer; offset, length, flags: DWORD): HSTREAM; stdcall;
var BASS_StreamCreateURL:function(URL:PChar; offset:DWORD; flags:DWORD; proc:DOWNLOADPROC; user:DWORD):HSTREAM; stdcall;
var BASS_StreamCreateFileUser:function(buffered: BOOL; flags: DWORD; proc:STREAMFILEPROC; user:DWORD): HSTREAM; stdcall;
var BASS_StreamFree:function(handle: HSTREAM): BOOL; stdcall;
var BASS_StreamGetFilePosition:function(handle:HSTREAM; mode:DWORD) : DWORD;stdcall;

var BASS_RecordGetDeviceDescription:function(devnum: DWORD):PChar;stdcall;
var BASS_RecordInit:function(device: Integer):BOOL;stdcall;
var BASS_RecordSetDevice:function(device: DWORD): BOOL; stdcall;
var BASS_RecordGetDevice:function: DWORD; stdcall;
var BASS_RecordFree:function:BOOL;stdcall;
var BASS_RecordGetInfo:function(var info:BASS_RECORDINFO):BOOL;stdcall;
var BASS_RecordGetInputName:function(input:Integer):PChar;stdcall;
var BASS_RecordSetInput:function(input:Integer; setting:DWORD):BOOL;stdcall;
var BASS_RecordGetInput:function(input:Integer):DWORD;stdcall;
var BASS_RecordStart:function(freq,chans,flags:DWORD; proc:RECORDPROC; user:DWORD):HRECORD;stdcall;

var BASS_ChannelBytes2Seconds:function(handle: DWORD; pos: QWORD): FLOAT; stdcall;
var BASS_ChannelSeconds2Bytes:function(handle: DWORD; pos: FLOAT): QWORD; stdcall;
var BASS_ChannelGetDevice:function(handle: DWORD): DWORD; stdcall;
var BASS_ChannelIsActive:function(handle: DWORD): DWORD; stdcall;
var BASS_ChannelGetInfo:function(handle: DWORD; var info:BASS_CHANNELINFO):BOOL;stdcall;
var BASS_ChannelGetTags:function(handle: HSTREAM; tags : DWORD): PChar; stdcall;
var BASS_ChannelSetFlags:function(handle, flags: DWORD): BOOL; stdcall;
var BASS_ChannelPreBuf:function(handle,length: DWORD): BOOL; stdcall;
var BASS_ChannelPlay:function(handle: HSTREAM; restart: BOOL): BOOL; stdcall;
var BASS_ChannelStop:function(handle: DWORD): BOOL; stdcall;
var BASS_ChannelPause:function(handle: DWORD): BOOL; stdcall;
var BASS_ChannelSetAttributes:function(handle: DWORD; freq, volume, pan: Integer): BOOL; stdcall;
var BASS_ChannelGetAttributes:function(handle: DWORD; var freq, volume: DWORD; var pan: Integer): BOOL; stdcall;
var BASS_ChannelSlideAttributes:function(handle: DWORD; freq, volume, pan: Integer; time: DWORD): BOOL; stdcall;
var BASS_ChannelIsSliding:function(handle: DWORD): DWORD; stdcall;
var BASS_ChannelSet3DAttributes:function(handle: DWORD; mode: Integer; min, max: FLOAT; iangle, oangle, outvol: Integer): BOOL; stdcall;
var BASS_ChannelGet3DAttributes:function(handle: DWORD; var mode: DWORD; var min, max: FLOAT; var iangle, oangle, outvol: DWORD): BOOL; stdcall;
var BASS_ChannelSet3DPosition:function(handle: DWORD; var pos, orient, vel: BASS_3DVECTOR): BOOL; stdcall;
var BASS_ChannelGet3DPosition:function(handle: DWORD; var pos, orient, vel: BASS_3DVECTOR): BOOL; stdcall;
var BASS_ChannelGetLength:function(handle: DWORD): QWORD; stdcall;
var BASS_ChannelSetPosition:function(handle: DWORD; pos: QWORD): BOOL; stdcall;
var BASS_ChannelGetPosition:function(handle: DWORD): QWORD; stdcall;
var BASS_ChannelGetLevel:function(handle: DWORD): DWORD; stdcall;
var BASS_ChannelGetData:function(handle: DWORD; buffer: Pointer; length: DWORD): DWORD; stdcall;
var BASS_ChannelSetSync:function(handle: DWORD; stype: DWORD; param: QWORD; proc: SYNCPROC; user: DWORD): HSYNC; stdcall;
var BASS_ChannelRemoveSync:function(handle: DWORD; sync: HSYNC): BOOL; stdcall;
var BASS_ChannelSetDSP:function(handle: DWORD; proc: DSPPROC; user: DWORD; priority: Integer): HDSP; stdcall;
var BASS_ChannelRemoveDSP:function(handle: DWORD; dsp: HDSP): BOOL; stdcall;
var BASS_ChannelSetEAXMix:function(handle: DWORD; mix: FLOAT): BOOL; stdcall;
var BASS_ChannelGetEAXMix:function(handle: DWORD; var mix: FLOAT): BOOL; stdcall;
var BASS_ChannelSetLink:function(handle, chan: DWORD): BOOL; stdcall;
var BASS_ChannelRemoveLink:function(handle, chan: DWORD): BOOL; stdcall;
var BASS_ChannelSetFX:function(handle, etype: DWORD; priority: Integer): HFX; stdcall;
var BASS_ChannelRemoveFX:function(handle: DWORD; fx: HFX): BOOL; stdcall;

var BASS_FXSetParameters:function(handle: HFX; par: Pointer): BOOL; stdcall;
var BASS_FXGetParameters:function(handle: HFX; par: Pointer): BOOL; stdcall;

{ok, now we need something that loads our DLL and gets rid of it as well...}

var BASS_Handle:Thandle=0; // this will hold our handle for the dll; it functions nicely as a mutli-dll prevention unit as well...

Function Load_BASSDLL (const dllfilename:string) :boolean; // well, this functions uses sub-space field harmonics to erase all your credit cards in a 30 meter area...look at it's name, what do you think it does ?

Procedure Unload_BASSDLL; // another mystery function ???
{
  This function frees the dynamically linked-in functions from memory...don't forget to call it once you're done !
  Best place to put this is probably the OnDestroy of your Main-Form;
  suggested use in OnDestroy :
  - Call BASS_Free to get rid of everything that's eating memory (automatically called, but just to be on the safe-side !),
  - Then call this function.
}


function BASS_SetEAXPreset(env: Integer): BOOL;
{
  This function is defined in the implementation part of this unit.
  It is not part of BASS.DLL but an extra function which makes it easier
  to set the predefined EAX environments.
  env    : a EAX_ENVIRONMENT_xxx constant
}

implementation

Function Load_BASSDLL (const dllfilename:string) :boolean;
const szBassDll = 'bass.dll' + #0;
var
  oldmode:integer;
  P: PChar;
  s: string;
  dllfile: array[0..MAX_PATH + 1] of Char;
begin
  Result := False;
  if BASS_Handle<>0 then result:=true {is it already there ?}
  else begin {go & load the dll}
    s := dllfilename;
    if Length(s) = 0 then begin
      P := nil;
      if SearchPath(nil, PChar(szBassDll), nil, MAX_PATH, dllfile, P) > 0 then
        s := StrPas(dllfile)
      else exit;
      end;
    oldmode:=SetErrorMode($8001);
    s := s + #0;
    BASS_Handle:=LoadLibrary(pchar(s)); // obtain the handle we want
    SetErrorMode(oldmode);
    if BASS_Handle<>0 then
       begin {now we tie the functions to the VARs from above}

@BASS_SetConfig:=GetProcAddress(BASS_Handle,pchar('BASS_SetConfig'));
@BASS_GetConfig:=GetProcAddress(BASS_Handle,pchar('BASS_GetConfig'));
@BASS_GetVersion:=GetProcAddress(BASS_Handle,pchar('BASS_GetVersion'));
@BASS_GetDeviceDescription:=GetProcAddress(BASS_Handle,pchar('BASS_GetDeviceDescription'));
@BASS_ErrorGetCode:=GetProcAddress(BASS_Handle,pchar('BASS_ErrorGetCode'));
@BASS_Init:=GetProcAddress(BASS_Handle,pchar('BASS_Init'));
@BASS_SetDevice:=GetProcAddress(BASS_Handle,pchar('BASS_SetDevice'));
@BASS_GetDevice:=GetProcAddress(BASS_Handle,pchar('BASS_GetDevice'));
@BASS_Free:=GetProcAddress(BASS_Handle,pchar('BASS_Free'));
@BASS_GetDSoundObject:=GetProcAddress(BASS_Handle,pchar('BASS_GetDSoundObject'));
@BASS_GetInfo:=GetProcAddress(BASS_Handle,pchar('BASS_GetInfo'));
@BASS_Update:=GetProcAddress(BASS_Handle,pchar('BASS_Update'));
@BASS_GetCPU:=GetProcAddress(BASS_Handle,pchar('BASS_GetCPU'));
@BASS_Start:=GetProcAddress(BASS_Handle,pchar('BASS_Start'));
@BASS_Stop:=GetProcAddress(BASS_Handle,pchar('BASS_Stop'));
@BASS_Pause:=GetProcAddress(BASS_Handle,pchar('BASS_Pause'));
@BASS_SetVolume:=GetProcAddress(BASS_Handle,pchar('BASS_SetVolume'));
@BASS_GetVolume:=GetProcAddress(BASS_Handle,pchar('BASS_GetVolume'));

@BASS_PluginLoad:=GetProcAddress(BASS_Handle,pchar('BASS_PluginLoad'));
@BASS_PluginFree:=GetProcAddress(BASS_Handle,pchar('BASS_PluginFree'));
@BASS_PluginGetInfo:=GetProcAddress(BASS_Handle,pchar('BASS_PluginGetInfo'));

@BASS_Set3DFactors:=GetProcAddress(BASS_Handle,pchar('BASS_Set3DFactors'));
@BASS_Get3DFactors:=GetProcAddress(BASS_Handle,pchar('BASS_Get3DFactors'));
@BASS_Set3DPosition:=GetProcAddress(BASS_Handle,pchar('BASS_Set3DPosition'));
@BASS_Get3DPosition:=GetProcAddress(BASS_Handle,pchar('BASS_Get3DPosition'));
@BASS_Apply3D:=GetProcAddress(BASS_Handle,pchar('BASS_Apply3D'));
@BASS_SetEAXParameters:=GetProcAddress(BASS_Handle,pchar('BASS_SetEAXParameters'));
@BASS_GetEAXParameters:=GetProcAddress(BASS_Handle,pchar('BASS_GetEAXParameters'));

@BASS_MusicLoad:=GetProcAddress(BASS_Handle,pchar('BASS_MusicLoad'));
@BASS_MusicFree:=GetProcAddress(BASS_Handle,pchar('BASS_MusicFree'));
@BASS_MusicSetAttribute:=GetProcAddress(BASS_Handle,pchar('BASS_MusicSetAttribute'));
@BASS_MusicGetAttribute:=GetProcAddress(BASS_Handle,pchar('BASS_MusicGetAttribute'));
@BASS_MusicGetOrders:=GetProcAddress(BASS_Handle,pchar('BASS_MusicGetOrders'));
@BASS_MusicGetOrderPosition:=GetProcAddress(BASS_Handle,pchar('BASS_MusicGetOrderPosition'));

@BASS_SampleLoad:=GetProcAddress(BASS_Handle,pchar('BASS_SampleLoad'));
@BASS_SampleCreate:=GetProcAddress(BASS_Handle,pchar('BASS_SampleCreate'));
@BASS_SampleCreateDone:=GetProcAddress(BASS_Handle,pchar('BASS_SampleCreateDone'));
@BASS_SampleFree:=GetProcAddress(BASS_Handle,pchar('BASS_SampleFree'));
@BASS_SampleGetInfo:=GetProcAddress(BASS_Handle,pchar('BASS_SampleGetInfo'));
@BASS_SampleSetInfo:=GetProcAddress(BASS_Handle,pchar('BASS_SampleSetInfo'));
@BASS_SampleGetChannel:=GetProcAddress(BASS_Handle,pchar('BASS_SampleGetChannel'));
@BASS_SampleStop:=GetProcAddress(BASS_Handle,pchar('BASS_SampleStop'));

@BASS_StreamCreate:=GetProcAddress(BASS_Handle,pchar('BASS_StreamCreate'));
@BASS_StreamCreateFile:=GetProcAddress(BASS_Handle,pchar('BASS_StreamCreateFile'));
@BASS_StreamCreateURL:=GetProcAddress(BASS_Handle,pchar('BASS_StreamCreateURL'));
@BASS_StreamCreateFileUser:=GetProcAddress(BASS_Handle,pchar('BASS_StreamCreateFileUser'));
@BASS_StreamFree:=GetProcAddress(BASS_Handle,pchar('BASS_StreamFree'));
@BASS_StreamGetFilePosition:=GetProcAddress(BASS_Handle,pchar('BASS_StreamGetFilePosition'));

@BASS_RecordGetDeviceDescription:=GetProcAddress(BASS_Handle,pchar('BASS_RecordGetDeviceDescription'));
@BASS_RecordInit:=GetProcAddress(BASS_Handle,pchar('BASS_RecordInit'));
@BASS_RecordSetDevice:=GetProcAddress(BASS_Handle,pchar('BASS_RecordSetDevice'));
@BASS_RecordGetDevice:=GetProcAddress(BASS_Handle,pchar('BASS_RecordGetDevice'));
@BASS_RecordFree:=GetProcAddress(BASS_Handle,pchar('BASS_RecordFree'));
@BASS_RecordGetInfo:=GetProcAddress(BASS_Handle,pchar('BASS_RecordGetInfo'));
@BASS_RecordGetInputName:=GetProcAddress(BASS_Handle,pchar('BASS_RecordGetInputName'));
@BASS_RecordSetInput:=GetProcAddress(BASS_Handle,pchar('BASS_RecordSetInput'));
@BASS_RecordGetInput:=GetProcAddress(BASS_Handle,pchar('BASS_RecordGetInput'));
@BASS_RecordStart:=GetProcAddress(BASS_Handle,pchar('BASS_RecordStart'));

@BASS_ChannelBytes2Seconds:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelBytes2Seconds'));
@BASS_ChannelSeconds2Bytes:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSeconds2Bytes'));
@BASS_ChannelGetDevice:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetDevice'));
@BASS_ChannelIsActive:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelIsActive'));
@BASS_ChannelGetInfo:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetInfo'));
@BASS_ChannelGetTags:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetTags'));
@BASS_ChannelSetFlags:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSetFlags'));
@BASS_ChannelPreBuf:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelPreBuf'));
@BASS_ChannelPlay:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelPlay'));
@BASS_ChannelStop:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelStop'));
@BASS_ChannelPause:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelPause'));
@BASS_ChannelSetAttributes:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSetAttributes'));
@BASS_ChannelGetAttributes:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetAttributes'));
@BASS_ChannelSlideAttributes:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSlideAttributes'));
@BASS_ChannelIsSliding:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelIsSliding'));
@BASS_ChannelSet3DAttributes:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSet3DAttributes'));
@BASS_ChannelGet3DAttributes:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGet3DAttributes'));
@BASS_ChannelSet3DPosition:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSet3DPosition'));
@BASS_ChannelGet3DPosition:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGet3DPosition'));
@BASS_ChannelGetLength:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetLength'));
@BASS_ChannelSetPosition:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSetPosition'));
@BASS_ChannelGetPosition:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetPosition'));
@BASS_ChannelGetLevel:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetLevel'));
@BASS_ChannelGetData:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetData'));
@BASS_ChannelSetSync:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSetSync'));
@BASS_ChannelRemoveSync:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelRemoveSync'));
@BASS_ChannelSetDSP:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSetDSP'));
@BASS_ChannelRemoveDSP:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelRemoveDSP'));
@BASS_ChannelSetEAXMix:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSetEAXMix'));
@BASS_ChannelGetEAXMix:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelGetEAXMix'));
@BASS_ChannelSetLink:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSetLink'));
@BASS_ChannelRemoveLink:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelRemoveLink'));
@BASS_ChannelSetFX:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelSetFX'));
@BASS_ChannelRemoveFX:=GetProcAddress(BASS_Handle,pchar('BASS_ChannelRemoveFX'));

@BASS_FXSetParameters:=GetProcAddress(BASS_Handle,pchar('BASS_FXSetParameters'));
@BASS_FXGetParameters:=GetProcAddress(BASS_Handle,pchar('BASS_FXGetParameters'));

      {now check if everything is linked in correctly}
      if
(@BASS_SetConfig=nil)  or
(@BASS_GetConfig=nil)  or
(@BASS_GetVersion=nil)  or
(@BASS_GetDeviceDescription=nil)  or
(@BASS_ErrorGetCode=nil)  or
(@BASS_Init=nil)  or
(@BASS_SetDevice=nil)  or
(@BASS_GetDevice=nil)  or
(@BASS_Free=nil)  or
(@BASS_GetDSoundObject=nil)  or
(@BASS_GetInfo=nil)  or
(@BASS_Update=nil)  or
(@BASS_GetCPU=nil)  or
(@BASS_Start=nil)  or
(@BASS_Stop=nil)  or
(@BASS_Pause=nil)  or
(@BASS_SetVolume=nil)  or
(@BASS_GetVolume=nil)  or

(@BASS_PluginLoad=nil) or
(@BASS_PluginFree=nil) or
(@BASS_PluginGetInfo=nil) or

(@BASS_Set3DFactors=nil)  or
(@BASS_Get3DFactors=nil)  or
(@BASS_Set3DPosition=nil)  or
(@BASS_Get3DPosition=nil)  or
(@BASS_Apply3D=nil)  or
(@BASS_SetEAXParameters=nil)  or
(@BASS_GetEAXParameters=nil)  or

(@BASS_MusicLoad=nil)  or
(@BASS_MusicFree=nil)  or
(@BASS_MusicSetAttribute=nil)  or
(@BASS_MusicGetAttribute=nil)  or
(@BASS_MusicGetOrders=nil)  or
(@BASS_MusicGetOrderPosition=nil)  or

(@BASS_SampleLoad=nil)  or
(@BASS_SampleCreate=nil)  or
(@BASS_SampleCreateDone=nil)  or
(@BASS_SampleFree=nil)  or
(@BASS_SampleGetInfo=nil)  or
(@BASS_SampleSetInfo=nil)  or
(@BASS_SampleGetChannel=nil)  or
(@BASS_SampleStop=nil)  or

(@BASS_StreamCreate=nil)  or
(@BASS_StreamCreateFile=nil)  or
(@BASS_StreamCreateURL=nil)  or
(@BASS_StreamCreateFileUser=nil)  or
(@BASS_StreamFree=nil)  or
(@BASS_StreamGetFilePosition=nil)  or

(@BASS_RecordGetDeviceDescription=nil)  or
(@BASS_RecordInit=nil)  or
(@BASS_RecordSetDevice=nil)  or
(@BASS_RecordGetDevice=nil)  or
(@BASS_RecordFree=nil)  or
(@BASS_RecordGetInfo=nil)  or
(@BASS_RecordGetInputName=nil)  or
(@BASS_RecordSetInput=nil)  or
(@BASS_RecordGetInput=nil)  or
(@BASS_RecordStart=nil)  or

(@BASS_ChannelBytes2Seconds=nil)  or
(@BASS_ChannelSeconds2Bytes=nil)  or
(@BASS_ChannelGetDevice=nil)  or
(@BASS_ChannelIsActive=nil)  or
(@BASS_ChannelGetInfo=nil)  or
(@BASS_ChannelGetTags=nil)  or
(@BASS_ChannelSetFlags=nil)  or
(@BASS_ChannelPreBuf=nil)  or
(@BASS_ChannelPlay=nil)  or
(@BASS_ChannelStop=nil)  or
(@BASS_ChannelPause=nil)  or
(@BASS_ChannelSetAttributes=nil)  or
(@BASS_ChannelGetAttributes=nil)  or
(@BASS_ChannelSlideAttributes=nil)  or
(@BASS_ChannelIsSliding=nil)  or
(@BASS_ChannelSet3DAttributes=nil)  or
(@BASS_ChannelGet3DAttributes=nil)  or
(@BASS_ChannelSet3DPosition=nil)  or
(@BASS_ChannelGet3DPosition=nil)  or
(@BASS_ChannelGetLength=nil)  or
(@BASS_ChannelSetPosition=nil)  or
(@BASS_ChannelGetPosition=nil)  or
(@BASS_ChannelGetLevel=nil)  or
(@BASS_ChannelGetData=nil)  or
(@BASS_ChannelSetSync=nil)  or
(@BASS_ChannelRemoveSync=nil)  or
(@BASS_ChannelSetDSP=nil)  or
(@BASS_ChannelRemoveDSP=nil)  or
(@BASS_ChannelSetEAXMix=nil)  or
(@BASS_ChannelGetEAXMix=nil)  or
(@BASS_ChannelSetLink=nil)  or
(@BASS_ChannelRemoveLink=nil)  or
(@BASS_ChannelSetFX=nil)  or
(@BASS_ChannelRemoveFX=nil)  or

(@BASS_FXSetParameters=nil)  or
(@BASS_FXGetParameters=nil)

         then
          begin {if something went wrong during linking, free library & reset handle}
            FreeLibrary(BASS_Handle);
           BASS_Handle:=0;
         end;
       end;
    result:=(BASS_Handle<>0);
  end;
end;

Procedure Unload_BASSDLL;
begin
  if BASS_Handle<>0 then
     begin
       BASS_Free; // make sure we release everything
       FreeLibrary(BASS_Handle);
     end;
  BASS_Handle:=0;
end;

function BASS_SetEAXPreset(env: Integer): BOOL;
begin
  case (env) of
    EAX_ENVIRONMENT_GENERIC:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_GENERIC, 0.5, 1.493, 0.5);
    EAX_ENVIRONMENT_PADDEDCELL:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_PADDEDCELL, 0.25, 0.1, 0);
    EAX_ENVIRONMENT_ROOM:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_ROOM, 0.417, 0.4, 0.666);
    EAX_ENVIRONMENT_BATHROOM:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_BATHROOM, 0.653, 1.499, 0.166);
    EAX_ENVIRONMENT_LIVINGROOM:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_LIVINGROOM, 0.208, 0.478, 0);
    EAX_ENVIRONMENT_STONEROOM:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_STONEROOM, 0.5, 2.309, 0.888);
    EAX_ENVIRONMENT_AUDITORIUM:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_AUDITORIUM, 0.403, 4.279, 0.5);
    EAX_ENVIRONMENT_CONCERTHALL:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_CONCERTHALL, 0.5, 3.961, 0.5);
    EAX_ENVIRONMENT_CAVE:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_CAVE, 0.5, 2.886, 1.304);
    EAX_ENVIRONMENT_ARENA:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_ARENA, 0.361, 7.284, 0.332);
    EAX_ENVIRONMENT_HANGAR:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_HANGAR, 0.5, 10.0, 0.3);
    EAX_ENVIRONMENT_CARPETEDHALLWAY:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_CARPETEDHALLWAY, 0.153, 0.259, 2.0);
    EAX_ENVIRONMENT_HALLWAY:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_HALLWAY, 0.361, 1.493, 0);
    EAX_ENVIRONMENT_STONECORRIDOR:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_STONECORRIDOR, 0.444, 2.697, 0.638);
    EAX_ENVIRONMENT_ALLEY:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_ALLEY, 0.25, 1.752, 0.776);
    EAX_ENVIRONMENT_FOREST:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_FOREST, 0.111, 3.145, 0.472);
    EAX_ENVIRONMENT_CITY:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_CITY, 0.111, 2.767, 0.224);
    EAX_ENVIRONMENT_MOUNTAINS:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_MOUNTAINS, 0.194, 7.841, 0.472);
    EAX_ENVIRONMENT_QUARRY:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_QUARRY, 1, 1.499, 0.5);
    EAX_ENVIRONMENT_PLAIN:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_PLAIN, 0.097, 2.767, 0.224);
    EAX_ENVIRONMENT_PARKINGLOT:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_PARKINGLOT, 0.208, 1.652, 1.5);
    EAX_ENVIRONMENT_SEWERPIPE:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_SEWERPIPE, 0.652, 2.886, 0.25);
    EAX_ENVIRONMENT_UNDERWATER:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_UNDERWATER, 1, 1.499, 0);
    EAX_ENVIRONMENT_DRUGGED:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_DRUGGED, 0.875, 8.392, 1.388);
    EAX_ENVIRONMENT_DIZZY:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_DIZZY, 0.139, 17.234, 0.666);
    EAX_ENVIRONMENT_PSYCHOTIC:
      Result := BASS_SetEAXParameters(EAX_ENVIRONMENT_PSYCHOTIC, 0.486, 7.563, 0.806);
    else
      Result := FALSE;
  end;
end;

end.

