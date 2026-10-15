#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "gif/lvgl_gif.h"
#include "lvgl_display.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <atomic>
#include <memory>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define PREVIEW_IMAGE_DURATION_MS 5000

class LcdDisplay : public LvglDisplay {
protected:
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;

    lv_draw_buf_t draw_buf_;
    lv_obj_t* top_bar_ = nullptr;
    lv_obj_t* status_bar_ = nullptr;
    lv_obj_t* content_ = nullptr;
    lv_obj_t* container_ = nullptr;
    lv_obj_t* side_bar_ = nullptr;
    lv_obj_t* bottom_bar_ = nullptr;
    lv_obj_t* preview_image_ = nullptr;
    lv_obj_t* emoji_label_ = nullptr;
    lv_obj_t* emoji_image_ = nullptr;
    std::unique_ptr<LvglGif> gif_controller_ = nullptr;
    lv_obj_t* emoji_box_ = nullptr;
    lv_obj_t* chat_message_label_ = nullptr;
    esp_timer_handle_t preview_timer_ = nullptr;
    std::unique_ptr<LvglImage> preview_image_cached_ = nullptr;
    bool hide_subtitle_ = false;  // Control whether to hide chat messages/subtitles

    // Big Character & dynamic stroke animation
    lv_obj_t* big_char_container_ = nullptr;
    lv_obj_t* big_char_pinyin_label_ = nullptr;
    lv_obj_t* big_char_tianzige_ = nullptr;
    lv_obj_t* big_char_label_ = nullptr;
    lv_obj_t* big_char_progress_label_ = nullptr;
    lv_obj_t* stroke_canvas_ = nullptr;
    uint32_t* stroke_canvas_buf_ = nullptr;
    esp_timer_handle_t big_char_timer_ = nullptr;
    esp_timer_handle_t stroke_timer_ = nullptr;
    esp_timer_handle_t stroke_stage2_timer_ = nullptr;
    TaskHandle_t stroke_fetch_task_ = nullptr;
    int64_t big_char_show_time_ = 0;

    std::vector<Stroke> stroke_data_;
    bool stroke_data_ready_ = false;
    int current_stroke_idx_ = 0;
    float current_stroke_progress_ = 0.0f;
    int stroke_pause_ticks_ = 0;

    // Pomodoro timer
    enum class PomodoroState { kIdle, kFocus, kBreak };
    PomodoroState pomodoro_state_ = PomodoroState::kIdle;
    int pomodoro_focus_sec_ = 25 * 60;
    int pomodoro_break_sec_ = 5 * 60;
    int pomodoro_remaining_sec_ = 0;
    esp_timer_handle_t pomodoro_timer_ = nullptr;
    lv_obj_t* pomodoro_badge_ = nullptr;
    lv_obj_t* pomodoro_label_ = nullptr;

    void OnStrokeTick();
    void StartStrokeAnimation();
    void StopStrokeAnimation();
    void OnPomodoroTick();

    void InitializeLcdThemes();
    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;

protected:
    // Add protected constructor
    LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width,
               int height);

public:
    struct StrokePoint { int16_t x; int16_t y; };
    using Stroke = std::vector<StrokePoint>;

    ~LcdDisplay();
    virtual void SetStatus(const char* status) override;
    virtual void SetEmotion(const char* emotion) override;
    virtual void ShowBigCharacter(const std::string& ch, const std::string& pinyin = "") override;
    virtual void DismissBigCharacter();
    virtual bool StartPomodoro(int focus_minutes = 25, int break_minutes = 5) override;
    virtual bool StopPomodoro() override;
    virtual void SetChatMessage(const char* role, const char* content) override;
    virtual void ClearChatMessages() override;
    virtual void SetPreviewImage(std::unique_ptr<LvglImage> image) override;
    virtual void SetupUI() override;
    // Add theme switching function
    virtual void SetTheme(Theme* theme) override;

    // Set whether to hide chat messages/subtitles
    virtual void SetHideSubtitle(bool hide) override;
};

// SPI LCD display
class SpiLcdDisplay : public LcdDisplay {
public:
    SpiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width,
                  int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y,
                  bool swap_xy);
};

// RGB LCD display
class RgbLcdDisplay : public LcdDisplay {
public:
    RgbLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width,
                  int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y,
                  bool swap_xy);
};

// MIPI LCD display
class MipiLcdDisplay : public LcdDisplay {
public:
    MipiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width,
                   int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y,
                   bool swap_xy);
};

#endif  // LCD_DISPLAY_H
