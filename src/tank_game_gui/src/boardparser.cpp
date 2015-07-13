/******************************************************************************
*
* Copyright Consoden AB, 2015
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
    ,m_coins()
	,m_laser_ammo()
    ,m_poison()
	,m_dudes()
	,m_smoke()

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
    ,m_coins()
	,m_laser_ammo()
    ,m_poison()
	,m_dudes()
	,m_smoke()

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
    for (const auto& pos : m_coins)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='$';
    }
    for (const auto& pos : m_poison)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='p';
    }
    for (const auto& pos : m_dudes)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='d';
    }

    for (const auto& pos : m_laser_ammo)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='l';
    }

    for (const auto& pos : m_smoke)
    {
        size_t index=static_cast<size_t>(pos.y()*m_xSize+pos.x());
        bin[index]='s';
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

/*bool Board::IsWall(const char* data, int x, int y){
	size_t index=static_cast<size_t>(y*m_xSize+x);
	return data[index]  == 'x';
}*/

const bool Board::isWall(qreal x, qreal y) const{
	for(auto p : m_walls){
		if(p.x() == x && p.y() == y){
			return true;
		}
	}

	return false;

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

            case 'd': //mine
            {
                m_dudes.push_back(QPointF(x, y));
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
                
            case '$': //coin
            {
                m_coins.push_back(QPointF(x, y));
            }
                break;

            case 'p': //poison
            {
                m_poison.push_back(QPointF(x, y));
            }
                break;

            case 'l': //poison
            {
                m_laser_ammo.push_back(QPointF(x, y));
            }
                break;

            case 's': //poison
            {
                m_smoke.push_back(QPointF(x, y));
            }
                break;


            case '.': //empty square
                break;
            }
        }
    }
}
