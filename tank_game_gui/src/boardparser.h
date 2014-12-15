/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef BOARDPARSER_H
#define BOARDPARSER_H

#include <QPoint>
#include <vector>
#include <string>

class Board
{
public:
    //types
    typedef std::vector<QPointF> PointVec;

    /**
     * @brief BoardParser - The parsing takes place in the constructor
     * @param filepath - Absolute path to file
     */
    Board(const std::string& filepath);

    /**
     * @brief BoardParser - The parsing takes place in the constructor
     * @param binary - pointer to data
     * @param xSize - board size X
     * @param ySize - borad size Y
     */
    Board(const char* binary, int xSize, int ySize);

    //Get parse result
    //---------------------
    int GetXSize() const {return m_xSize;}
    int GetYSize() const {return m_ySize;}

    const PointVec& Tanks() const {return m_tanks;}
    const PointVec& Walls() const {return m_walls;}
    const PointVec& Mines() const {return m_mines;}
    const PointVec& Flags() const {return m_flags;}

    void ReverseTanks();
    void ToBinary(std::vector<char>& bin) const;    
    void Save(const std::string& filepath) const;

private:
    int m_xSize;
    int m_ySize;
    PointVec m_tanks;
    PointVec m_walls;
    PointVec m_mines;
    PointVec m_flags;

    void Parse(const char* data);
};

#endif
