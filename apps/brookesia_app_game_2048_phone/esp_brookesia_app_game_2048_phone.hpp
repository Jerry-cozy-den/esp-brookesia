/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "lvgl.h"
#include "systems/phone/esp_brookesia_phone_app.hpp"

namespace esp_brookesia::apps {

typedef struct {
    int x; // Index of row
    int y; // Index of column
    int weight;
} cell_weight_t;

class Game2048: public ESP_Brookesia_PhoneApp {
public:
    /**
     * @brief Get the singleton instance of Game2048
     *
     * @param use_status_bar Flag to show the status bar
     * @param use_navigation_bar Flag to show the navigation bar
     * @return Pointer to the singleton instance
     */
    static Game2048 *requestInstance(bool use_status_bar = false, bool use_navigation_bar = false);

    /**
     * @brief Destructor for the Game2048 app
     *
     */
    ~Game2048();

    using ESP_Brookesia_PhoneApp::startRecordResource;
    using ESP_Brookesia_PhoneApp::endRecordResource;

protected:
    /**
     * @brief Private constructor to enforce singleton pattern
     *
     * @param use_status_bar Flag to show the status bar
     * @param use_navigation_bar Flag to show the navigation bar
     */
    Game2048(bool use_status_bar, bool use_navigation_bar);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// The following functions must be implemented by the user's app class. /////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief Called when the app starts running. This is the entry point for the app, where all UI resources should be
     *        created.
     *
     * @return true if success, otherwise false.
     *
     */
    bool run(void) override;

    /**
     * @brief Called when the app is requested to go back. This is usually triggered by the back button or gesture.
     *
     * @return true if the back operation is handled by the app, otherwise false. If false is returned, the system will
     *         handle the back operation.
     *
     */
    bool back(void) override;

    /**
     * @brief Called when the app is requested to close. This is usually triggered by the close button or gesture.
     *
     * @return true if success, otherwise false.
     *
     */
    bool close(void) override;

    // Game logic methods
    void debugCells();
    void debugCells(int cell[4][4]);
    void debugCells(lv_obj_t *cell[4][4]);
    void debugCells(cell_weight_t cell[4][4]);
    void debugCells(lv_obj_t *cell[4]);
    void cleanForegroundCells();
    void generateForegroundCell();
    void addRemoveReadyCell(lv_obj_t *cell);
    void cleanRemoveReadyCell();
    void newGame();
    void updateCellValue();
    void updateCurrentScore(int score);
    void updateBestScore(int score);
    void updateCellsStyle();
    int maxWeight();
    int moveLeft();
    int moveRight();
    int moveUp();
    int moveDown();
    bool isGameOver();

private:
    static Game2048 *_instance;

    lv_obj_t *addBackgroundCell(lv_obj_t *parent);
    void startAnimationX(lv_obj_t *target, int x, int time);
    void startAnimationY(lv_obj_t *target, int y, int time);

    static void new_game_event_cb(lv_event_t *e);
    static void motion_event_cb(lv_event_t *e);
    static void anim_finish_cb(lv_anim_t *a);

    uint16_t _width = 0;
    uint16_t _height = 0;
    uint16_t _current_score = 0;
    uint16_t _best_score = 0;
    uint16_t _weight_max = 0;
    bool _is_closing = false;
    bool _anim_running_flag = false;
    bool _generate_cell_flag = false;

    cell_weight_t _cells_weight[4][4] = {};
    lv_obj_t *_cur_score_label = nullptr;
    lv_obj_t *_best_score_label = nullptr;
    lv_obj_t *_background_cells[4][4] = {};
    lv_obj_t *_foreground_cells[4][4] = {};
    lv_obj_t *_remove_ready_cells[4][4] = {};
    lv_obj_t *_foreground_grid = nullptr;
    lv_obj_t *_game_grid = nullptr;
    lv_color_t _cell_colors[11] = {};
};

} // namespace esp_brookesia::apps
