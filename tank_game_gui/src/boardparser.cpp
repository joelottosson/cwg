/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <fstream>
#include "boardparser.h"

Board::Board(const std::string& filepath)
    :m_xSize(0)
    ,m_ySize(0)
    ,m_tanks()
    ,m_walls()
    ,m_mines()
    ,m_flags()
    ,m_poison()
{
    std::ifstream is;
    is.open(filepath.c_str());

    std::vector<char> bin;
    bin.reserve(1000);

    std::string line;
    while (std::getline(is, line))
    {
        if (line.empty())
            break;

        if (m_xSize==0)
        {
            m_xSize=line.size();
        }
        else if (static_cast<int>(line.size())!=m_xSize)
        {
            throw "Invalid game board size, all rows must have the same size!";
        }

        bin.insert(bin.end(), line.begin(), line.end());
        ++m_ySize;
    }

    is.close();

    Parse(&bin[0]);
}

Board::Board(const char* binary, int xSize, int ySize)
    :m_xSize(xSize)
    ,m_ySize(ySize)
    ,m_tanks()
    ,m_walls()
    ,m_mines()
    ,m_flags()
    ,m_poison()
{
    Parse(binary);
}

void Board::ReverseTanks()
{
    PointVec tmp=m_tanks;
    m_tanks.clear();
    for (auto it=tmp.rbegin(); it!=tmp.rend(); ++it)
    {
        m_tanks.push_back(*it);
    }
}

void Board::ToBinary(std::vector<char>& bin) const
{
    bin.clear();
    bin.resize(m_xSize*m_ySize, '.');
    for (const auto& pos : m_walls)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='x';
    }
    for (const auto& pos : m_mines)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='o';
    }
    for (const auto& pos : m_flags)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='f';
    }
    for (const auto& pos : m_poison)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='p';
    }
}

void Board::Save(const std::string& filepath) const
{
    std::vector<char> bin;
    ToBinary(bin);
    for (const auto& pos : m_tanks)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='t';
    }

    std::ofstream os;
    os.open(filepath.c_str());
    for (size_t i=0; i<bin.size(); ++i)
    {
        os<<bin[i];
        if ((i+1)%m_xSize==0)
        {
            os<<std::endl;
        }
    }
    os.close();
}

void Board::Parse(const char* data)
{
    for (int y=0; y<m_ySize; ++y)
    {
        for (int x=0; x<m_xSize; ++x)
        {
            size_t index=static_cast<size_t>(y*m_xSize+x);

            switch (data[index])
            {
            case 'o': //mine
            {
                m_mines.push_back(QPointF(x, y));
            }
                break;

            case 'x': //wall
            {
                m_walls.push_back(QPointF(x, y));
            }
                break;

            case 't': //tank when reading from file, not present in gamestate
            {
                m_tanks.push_back(QPointF(x, y));
            }
                break;
                
            case 'f': //flag
            {
                m_flags.push_back(QPointF(x, y));
            }
                break;

            case 'p': //poison
            {
                m_poison.push_back(QPointF(x, y));
            }
                break;

            case '.': //empty square
                break;
            }
        }
    }
}
