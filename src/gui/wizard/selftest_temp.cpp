// selftest_temp.cpp
#include "selftest_temp.hpp"
#include "i18n.h"
#include "gui.hpp"

StateFncData StateFnc_SELFTEST_INIT_TEMP(StateFncData last_run) {
    static const char en_text[] = N_(
        "State\n"
        "SELFTEST_INIT_TEMP\n"
        "not implemented");
    const string_view_utf8 notTranslatedText = string_view_utf8::MakeCPUFLASH((const uint8_t *)(en_text));

    MsgBox(notTranslatedText, Responses_NEXT, 0, GuiDefaults::RectScreenBody, is_multiline::no);
    return last_run.PassToNext();
}

StateFncData StateFnc_SELFTEST_TEMP(StateFncData last_run) {
    static const char en_text[] = N_(
        "State\n"
        "SELFTEST_TEMP\n"
        "not implemented");
    const string_view_utf8 notTranslatedText = string_view_utf8::MakeCPUFLASH((const uint8_t *)(en_text));

    MsgBox(notTranslatedText, Responses_NEXT, 0, GuiDefaults::RectScreenBody, is_multiline::no);
    return last_run.PassToNext();
}

#if 0
    #include "config.h"
    #include "marlin_client.h"
    #include "wizard_config.hpp"
    #include "guitypes.hpp" //font_meas_text

struct selftest_temp_screen_t {
    window_text_t text_checking_temp;
    window_progress_t progress;

    uint32_t timer_noz;
    uint32_t timer_bed;
};

struct selftest_temp_data_t {
    _TEST_STATE_t state_preheat_nozzle;
    _TEST_STATE_t state_preheat_bed;
    _TEST_STATE_t state_temp_nozzle;
    _TEST_STATE_t state_temp_bed;
    float temp_noz;
    float temp_bed;
};

//-----------------------------------------------------------------------------
//function definitions
void _wizard_temp_actualize_temperatures(selftest_temp_data_t *p_data); //data will survive test

//static const char* _str_progress_preheat_noz = " 25   preheat   " STR(_CALIB_TEMP_NOZ);
//static const char* _str_progress_preheat_bed = " 25   preheat   " STR(_CALIB_TEMP_BED);

//static const char* _str_progress_heat_noz = "  " STR(_PASS_MIN_TEMP_NOZ) " heat   " STR(_PASS_MAX_TEMP_NOZ);
//static const char* _str_progress_heat_bed = "  " STR(_PASS_MIN_TEMP_BED) " heat   " STR(_PASS_MAX_TEMP_BED);
//-----------------------------------------------------------------------------
//function declarations
void wizard_init_screen_selftest_temp(int16_t id_body, selftest_temp_screen_t *p_screen, selftest_temp_data_t *p_data) {
    /*  _wizard_temp_actualize_temperatures(p_data);
    //	point_ui16_t pt,pt2;
    // window_destroy_children(id_body);
    window_t *pWin = window_ptr(id_body);
    if (pWin != 0)
        pWin->Show();
    pWin->Invalidate();

    uint16_t y = 40;
    uint16_t x = WIZARD_MARGIN_LEFT;
    uint16_t row_h = 22;

    window_create_ptr(WINDOW_CLS_TEXT, id_body, Rect16(x, y, WIZARD_X_SPACE, row_h * 2), &(p_screen->text_checking_temp));
    p_screen->text_checking_temp.SetText(_(
        "Checking hotend and\n"
        "heatbed heaters"));
    p_screen->text_checking_temp.SetAlignment(ALIGN_CENTER);

    y += row_h * 2;
    window_create_ptr(WINDOW_CLS_PROGRESS, id_body, Rect16(x, y, WIZARD_X_SPACE, 8), &(p_screen->progress));
*/
}

void _wizard_temp_actualize_temperatures(selftest_temp_data_t *p_data) {
    marlin_vars_t *vars = marlin_update_vars(MARLIN_VAR_MSK_TEMP_CURR);
    float t_noz = vars->temp_nozzle;
    float t_bed = vars->temp_bed;
    if (t_noz > p_data->temp_noz)
        p_data->temp_noz = t_noz;
    if (t_bed > p_data->temp_bed)
        p_data->temp_bed = t_bed;
}

//if timeout is reached, test fails
int _wizard_selftest_preheat(_TEST_STATE_t *state, uint32_t *p_timer,
    uint32_t temp_rq, int temp_ms, uint8_t marlin_var_id, uint32_t max_preheat_time_ms) {
    if (temp_ms >= int(temp_rq)) {
        *state = _TEST_PASSED;
        return 100;
    }

    int progress = wizard_timer(p_timer, max_preheat_time_ms, state, _WIZ_TIMER_AUTOFAIL);

    return progress;
}

int wizard_selftest_preheat_nozzle(int16_t id_body, selftest_temp_screen_t *p_screen, selftest_temp_data_t *p_data) {
    int ret = _wizard_selftest_preheat(
        &(p_data->state_preheat_nozzle),
        &p_screen->timer_noz,
        _CALIB_TEMP_NOZ,
        p_data->temp_noz,
        MARLIN_VAR_TEMP_NOZ,
        _MAX_PREHEAT_TIME_MS_NOZ);

    if (p_data->state_preheat_nozzle == _TEST_FAILED)
        marlin_gcode("M104 S0"); // nozzle temp 0

    return ret;
}

int wizard_selftest_preheat_bed(int16_t id_body, selftest_temp_screen_t *p_screen, selftest_temp_data_t *p_data) {
    int ret = _wizard_selftest_preheat(
        &(p_data->state_preheat_bed),
        &p_screen->timer_bed,
        _CALIB_TEMP_BED,
        p_data->temp_bed,
        MARLIN_VAR_TEMP_BED,
        _MAX_PREHEAT_TIME_MS_BED);

    if (p_data->state_preheat_bed == _TEST_FAILED)
        marlin_gcode("M140 S0"); // bed temp 0

    return ret;
}

int wizard_selftest_temp_nozzle(int16_t id_body, selftest_temp_screen_t *p_screen, selftest_temp_data_t *p_data) {
    if (_is_test_done(p_data->state_temp_nozzle))
        return 100;
    int progress = wizard_timer(&p_screen->timer_noz, _HEAT_TIME_MS_NOZ, &(p_data->state_temp_nozzle), _WIZ_TIMER);
    if (progress >= 99) {
        if ((p_data->temp_noz >= _PASS_MIN_TEMP_NOZ) && (p_data->temp_noz <= _PASS_MAX_TEMP_NOZ)) {
            p_data->state_temp_nozzle = _TEST_PASSED;
        } else {
            p_data->state_temp_nozzle = _TEST_FAILED;
        }

        marlin_gcode("M104 S0"); // nozzle temp 0
        progress = 100;
    }
    return progress;
}

int wizard_selftest_temp_bed(int16_t id_body, selftest_temp_screen_t *p_screen, selftest_temp_data_t *p_data) {
    if (_is_test_done(p_data->state_temp_bed))
        return 100;
    int progress = wizard_timer(&p_screen->timer_bed, _HEAT_TIME_MS_BED, &(p_data->state_temp_bed), _WIZ_TIMER);
    if (progress >= 99) {
        if ((p_data->temp_bed >= _PASS_MIN_TEMP_BED) && (p_data->temp_bed <= _PASS_MAX_TEMP_BED)) {
            p_data->state_temp_bed = _TEST_PASSED;
        } else {
            p_data->state_temp_bed = _TEST_FAILED;
        }

        marlin_gcode("M140 S0"); // bed temp 0
        progress = 100;
    }
    return progress;
}

int wizard_selftest_temp(int16_t id_body, selftest_temp_screen_t *p_screen, selftest_temp_data_t *p_data) {
    int progress_preheat_noz = 0;
    int progress_preheat_bed = 0;
    int progress_noz = 0;
    int progress_bed = 0;

    _wizard_temp_actualize_temperatures(p_data);

    if ((p_data->state_preheat_nozzle == _TEST_START) && (p_data->state_preheat_bed == _TEST_START)) {
        p_data->temp_noz = 0;
        p_data->temp_bed = 0;
        //turn heaters on
        // i should not reach those temeratures
        marlin_gcode_printf("M104 S%d", _MAX_TEMP_NOZ); // nozzle temp
        marlin_gcode_printf("M140 S%d", _MAX_TEMP_BED); // bed temp
        wizard_init_screen_selftest_temp(id_body, p_screen, p_data);
    }

    if (!_is_test_done(p_data->state_preheat_nozzle)) {
        progress_preheat_noz = wizard_selftest_preheat_nozzle(id_body, p_screen, p_data);
    } else {
        progress_preheat_noz = 100;
    }

    if (!_is_test_done(p_data->state_preheat_bed)) {
        progress_preheat_bed = wizard_selftest_preheat_bed(id_body, p_screen, p_data);
    } else {
        progress_preheat_bed = 100;
    }

    if (p_data->state_preheat_nozzle == _TEST_PASSED) {
        progress_noz = wizard_selftest_temp_nozzle(id_body, p_screen, p_data);
    }
    if (p_data->state_preheat_bed == _TEST_PASSED) {
        progress_bed = wizard_selftest_temp_bed(id_body, p_screen, p_data);
    }

    int progress = (progress_preheat_noz * _MAX_PREHEAT_TIME_MS_NOZ + progress_preheat_bed * _MAX_PREHEAT_TIME_MS_BED + progress_bed * _HEAT_TIME_MS_BED + progress_noz * _HEAT_TIME_MS_NOZ) / (_MAX_PREHEAT_TIME_MS_NOZ + _MAX_PREHEAT_TIME_MS_BED + _HEAT_TIME_MS_BED + _HEAT_TIME_MS_NOZ);

    if (p_data->state_preheat_nozzle == _TEST_FAILED)
        progress = 100;
    if (p_data->state_preheat_bed == _TEST_FAILED)
        progress = 100;
    if (p_data->state_temp_nozzle == _TEST_FAILED)
        progress = 100;
    if (p_data->state_temp_bed == _TEST_FAILED)
        progress = 100;

    p_screen->progress.SetValue(progress);
    if (progress == 100) {
        //turn heaters off
        marlin_gcode("M104 S0"); //nozzle temp 0
        marlin_gcode("M140 S0"); //bed temp 0
    }

    return progress;
}

#endif //0
