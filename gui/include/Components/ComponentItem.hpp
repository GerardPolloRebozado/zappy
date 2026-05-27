/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** ComponentItem.hpp
*/
#ifndef ZAPPY_GUI_COMPONENTITEM_HPP
#define ZAPPY_GUI_COMPONENTITEM_HPP


struct Position {
    int _x;
    int _y;
};


struct Items {
    enum Type { STONES, FOODS };
    enum StoneTypes {LINEMATE, DERAUMERE, SIBUR, MENDIANE, PHIRAS, THYSTAME};
    Type _current_type;
    StoneTypes _stone_type;
    Position pos;
};


#endif //ZAPPY_GUI_COMPONENTITEM_HPP
