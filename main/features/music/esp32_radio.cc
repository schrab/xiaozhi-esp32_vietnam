/**
 * @file esp32_radio.cc
 * @brief Internet radio player  supports AAC and MP3 streams.
 *
 * Inherits AudioStreamPlayer for streaming, decoding and playback.
 * Adds: preset station list, station search, volume per station,
 *       decoder type auto-detection.
 */

#include "esp32_radio.h"
#include "board.h"
#include "audio/audio_codec.h"

#include <esp_log.h>
#include <algorithm>
#include <cctype>

static const char* TAG = "Esp32Radio";

/* ================================================================== */
/*  Constructor / Destructor                                          */
/* ================================================================== */

Esp32Radio::Esp32Radio()
    : AudioStreamPlayer(),
      station_name_displayed_(false),
      current_station_volume_(RADIO_DEFAULT_VOLUME)
{
}

Esp32Radio::~Esp32Radio()
{
    ESP_LOGI(TAG, "Destroying Esp32Radio");
    Stop();
    ESP_LOGI(TAG, "Esp32Radio destroyed");
}

void Esp32Radio::Initialize(AudioCodec* codec)
{
    if (codec) {
        SetAudioCodec(codec);
    }
    InitializeRadioStations();
    ESP_LOGI(TAG, "Radio player initialised with %d stations (codec=%s)",
             (int)radio_stations_.size(), codec ? "direct" : "app-pipeline");
}

/* ================================================================== */
/*  Station presets                                                    */
/* ================================================================== */

void Esp32Radio::InitializeRadioStations()
{
    /* === Russian & Dance === */
    radio_stations_["RECORD"]         = RadioStation("Радио Рекорд",             "https://radiorecord.hostingradio.ru/rr_main96.aacp", "Главная танцевальная станция", "Dance / Electronic", 4.5f);
    radio_stations_["RUSSIAN_MIX"]    = RadioStation("Рекорд Русский Микс",      "https://radiorecord.hostingradio.ru/rus96.aacp",     "Русские танцевальные хиты",     "Russian Dance / Pop", 4.5f);
    radio_stations_["CHILL"]          = RadioStation("Рекорд Chill-Out",         "https://radiorecord.hostingradio.ru/chil96.aacp",    "Спокойная расслабляющая музыка", "Chillout / Lounge",   4.5f);
    radio_stations_["DFM"]            = RadioStation("DFM",                      "http://dfm.hostingradio.ru/dfm96.aacp",              "Клубная и танцевальная музыка",  "Club / Dance",        4.5f);
    radio_stations_["NASHE"]          = RadioStation("Наше Радио",               "http://nashe1.hostingradio.ru/nashe-128.mp3",       "Русский рок и рок-хиты",         "Russian Rock",        4.5f);
    radio_stations_["RECORD_ROCK"]    = RadioStation("Рекорд Рок",               "https://radiorecord.hostingradio.ru/rock96.aacp",    "Зарубежный и классический рок",  "Rock",                4.5f);

    /* === Ambient, Lo-Fi & Chill === */
    radio_stations_["CODERADIO"]      = RadioStation("CodeRadio (freeCodeCamp)", "https://coderadio-admin-v2.freecodecamp.org/listen/coderadio/radio.mp3", "Lo-Fi биты для работы и учёбы", "Lo-Fi / Beats", 4.5f);
    radio_stations_["GROOVE_SALAD"]   = RadioStation("SomaFM Groove Salad",      "https://ice2.somafm.com/groovesalad-128-mp3",       "Downtempo и чиллаут эмбиент",   "Ambient / Downtempo", 4.5f);
    radio_stations_["DRONE_ZONE"]     = RadioStation("SomaFM Drone Zone",        "https://ice2.somafm.com/dronezone-128-mp3",         "Атмосферный космический эмбиент", "Ambient / Space",    4.5f);
    radio_stations_["DEFCON"]         = RadioStation("SomaFM DEF CON",           "https://ice2.somafm.com/defcon-128-mp3",            "Хакерская электронная музыка",   "Synth / Electronic",  4.5f);
    radio_stations_["SECRET_AGENT"]   = RadioStation("SomaFM Secret Agent",      "https://ice2.somafm.com/secretagent-128-mp3",       "Шпионский лаунж и ретро",        "Lounge / Retro",      4.5f);

    /* === Jazz, Rock & Classical === */
    radio_stations_["JAZZ"]           = RadioStation("KSDS Jazz 88.3",           "https://ksds-ice.streamguys1.com/ksds.mp3",          "Классический и современный джаз", "Jazz",                4.5f);
    radio_stations_["PARADISE"]       = RadioStation("Radio Paradise",           "http://stream.radioparadise.com/mp3-128",            "Эклектичный рок, инди и акустика", "Eclectic / Rock",    4.5f);
    radio_stations_["CLASSICAL"]      = RadioStation("Classical California KUSC","https://14103.live.streamtheworld.com/KUSCAAC96.aac", "Симфоническая и классическая музыка", "Classical", 4.5f);
    radio_stations_["CINEMIX"]        = RadioStation("Cinemix",                  "https://kathy.torontocast.com:1825/stream",          "Музыка и саундтреки из фильмов", "Soundtracks",         4.5f);

    ESP_LOGI(TAG, "Initialised %d radio stations", (int)radio_stations_.size());
}

/* ================================================================== */
/*  Decoder-type heuristic                                            */
/* ================================================================== */

AudioDecoderType Esp32Radio::GuessDecoderType(const std::string& url) const
{
    std::string lower = url;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower.find(".aac") != std::string::npos ||
        lower.find("aacp") != std::string::npos ||
        lower.find("aac+") != std::string::npos ||
        lower.find("/aac") != std::string::npos ||
        lower.find("-aac") != std::string::npos) {
        return AudioDecoderType::AAC;
    }

    if (lower.find(".mp3") != std::string::npos ||
        lower.find("-mp3") != std::string::npos ||
        lower.find("/mp3") != std::string::npos) {
        return AudioDecoderType::MP3;
    }

    /* Default to MP3 for standard shoutcast/icecast streams */
    return AudioDecoderType::MP3;
}

/* ================================================================== */
/*  PlayStation  find station by name/key                            */
/* ================================================================== */

bool Esp32Radio::PlayStation(const std::string& station_name)
{
    ESP_LOGI(TAG, "PlayRadio: %s", station_name.c_str());

    std::string lower_input = station_name;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);

    /* 1) Search by display name (partial, case-insensitive) */
    for (const auto& kv : radio_stations_) {
        std::string lower_name = kv.second.name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

        if (lower_name.find(lower_input) != std::string::npos ||
            lower_input.find(lower_name) != std::string::npos) {
            ESP_LOGI(TAG, "Matched display name: '%s' -> %s (vol=%.1f)",
                     station_name.c_str(), kv.second.name.c_str(), kv.second.volume);
            current_station_volume_ = kv.second.volume;
            return PlayUrl(kv.second.url, kv.second.name);
        }
    }

    /* 2) Exact key match */
    auto it = radio_stations_.find(station_name);
    if (it != radio_stations_.end()) {
        current_station_volume_ = it->second.volume;
        return PlayUrl(it->second.url, it->second.name);
    }

    /* 3) Key match (case-insensitive) */
    for (const auto& kv : radio_stations_) {
        std::string lower_key = kv.first;
        std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
        if (lower_key == lower_input) {
            current_station_volume_ = kv.second.volume;
            return PlayUrl(kv.second.url, kv.second.name);
        }
    }

    /* 4) Russian and English keyword / genre matching */
    static const std::vector<std::pair<std::string, std::string>> keyword_map = {
        {"рекорд", "RECORD"},
        {"record", "RECORD"},
        {"русск", "RUSSIAN_MIX"},
        {"russian", "RUSSIAN_MIX"},
        {"чилл", "CHILL"},
        {"chill", "CHILL"},
        {"dfm", "DFM"},
        {"дифм", "DFM"},
        {"динамит", "DFM"},
        {"наше", "NASHE"},
        {"рок", "RECORD_ROCK"},
        {"rock", "RECORD_ROCK"},
        {"lofi", "CODERADIO"},
        {"lo-fi", "CODERADIO"},
        {"лофай", "CODERADIO"},
        {"code", "CODERADIO"},
        {"код", "CODERADIO"},
        {"salad", "GROOVE_SALAD"},
        {"салад", "GROOVE_SALAD"},
        {"drone", "DRONE_ZONE"},
        {"дроун", "DRONE_ZONE"},
        {"defcon", "DEFCON"},
        {"дефкон", "DEFCON"},
        {"agent", "SECRET_AGENT"},
        {"агент", "SECRET_AGENT"},
        {"jazz", "JAZZ"},
        {"джаз", "JAZZ"},
        {"парадайз", "PARADISE"},
        {"paradise", "PARADISE"},
        {"классик", "CLASSICAL"},
        {"classic", "CLASSICAL"},
        {"кино", "CINEMIX"},
        {"cinema", "CINEMIX"},
        {"саундтрек", "CINEMIX"},
        {"soundtrack", "CINEMIX"},
    };

    for (const auto& pair : keyword_map) {
        if (lower_input.find(pair.first) != std::string::npos) {
            auto found = radio_stations_.find(pair.second);
            if (found != radio_stations_.end()) {
                current_station_volume_ = found->second.volume;
                return PlayUrl(found->second.url, found->second.name);
            }
        }
    }

    /* 5) Fallback: if user just said "radio" or "any" or "random" */
    if (lower_input.empty() || lower_input == "random" || lower_input == "радио" ||
        lower_input == "рандом" || lower_input == "любое" || lower_input == "radio") {
        auto def = radio_stations_.find("RECORD");
        if (def != radio_stations_.end()) {
            current_station_volume_ = def->second.volume;
            return PlayUrl(def->second.url, def->second.name);
        }
    }

    ESP_LOGE(TAG, "Station not found: '%s'", station_name.c_str());
    return false;
}

/* ================================================================== */
/*  PlayUrl                                                           */
/* ================================================================== */

bool Esp32Radio::PlayUrl(const std::string& radio_url, const std::string& station_name)
{
    if (radio_url.empty()) {
        ESP_LOGE(TAG, "Radio URL is empty");
        return false;
    }

    ESP_LOGI(TAG, "PlayUrl: %s (%s)",
             station_name.empty() ? "Custom URL" : station_name.c_str(),
             radio_url.c_str());

    Stop();

    current_station_url_  = radio_url;
    current_station_name_ = station_name.empty() ? "Custom Radio" : station_name;
    station_name_displayed_ = false;

    if (current_station_volume_ <= 0.0f) {
        current_station_volume_ = RADIO_DEFAULT_VOLUME;
    }
    SetVolume(current_station_volume_);

    /* Auto-detect decoder type */
    AudioDecoderType dtype = GuessDecoderType(radio_url);
    ESP_LOGI(TAG, "Using decoder: %s", (dtype == AudioDecoderType::AAC) ? "AAC" : "MP3");

    return StartStream(radio_url, dtype);
}

/* ================================================================== */
/*  Stop                                                              */
/* ================================================================== */

bool Esp32Radio::Stop()
{
    if (!IsPlaying() && !IsDownloading()) {
        return true;
    }

    ESP_LOGI(TAG, "Stopping radio");
    return StopStream();
}

/* ================================================================== */
/*  GetStationList                                                    */
/* ================================================================== */

std::vector<std::string> Esp32Radio::GetStationList() const
{
    std::vector<std::string> list;
    list.reserve(radio_stations_.size());
    for (const auto& kv : radio_stations_) {
        list.push_back(kv.first + " - " + kv.second.name);
    }
    return list;
}

/* ================================================================== */
/*  AudioStreamPlayer hooks                                           */
/* ================================================================== */

void Esp32Radio::OnStreamInfoReady(int sample_rate, int bits_per_sample, int channels, int bitrate, int frame_size)
{
    ESP_LOGI(TAG, "Stream info: %s, %d Hz, %d bit, %d ch, %d kbps, %d frame size",
             current_station_name_.c_str(), sample_rate, bits_per_sample, channels, bitrate, frame_size);
}

void Esp32Radio::OnDisplayReady()
{
    /* Display is now handled externally via Application callbacks */
    ESP_LOGD(TAG, "Display ready for station: %s", current_station_name_.c_str());
    station_name_displayed_ = true;
}

bool Esp32Radio::OnPlaybackFinishedAndContinue()
{
    ESP_LOGI(TAG, "Radio playback finished");
    return false;
}
