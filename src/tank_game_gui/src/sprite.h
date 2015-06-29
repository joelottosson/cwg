/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef _TANK_GUI_SPRITE_H_
#define _TANK_GUI_SPRITE_H_

#include <QTime>
#include <QPixmap>
#include <qpixmap.h>
#include <iostream>

struct SpriteData
{
    QPixmap image;
    std::vector<QRectF> fragments;
    qint64 lifeTime; //sprite play time in millisecs
};

class Sprite
{
public:
    Sprite(const SpriteData& spriteData,
           const QPointF& position,
           qint64 startTime,
           int repetitions) //0 repetitions means forever
        :m_spriteData(&spriteData)
        ,m_currentFragment(0)
        ,m_pos(position)
        ,m_speed(0, 0)
        ,m_rotation(0)
        ,m_startTime(startTime)
        ,m_timeSlice(spriteData.lifeTime/spriteData.fragments.size())
        ,m_lastUpdated(QDateTime::currentMSecsSinceEpoch())
        ,m_repetitions(repetitions)
		,m_singleframe(false)
		,m_kill_instantly(false)
    {
    	if(repetitions < 0){m_singleframe = true;}
    }

    Sprite(const SpriteData& spriteData,
           const QPointF& position,
           qint64 startTime,
           int repetitions,//0 repetitions means forever
		   bool single) // Wether or not the sprite will only live for one update.
        :m_spriteData(&spriteData)
        ,m_currentFragment(0)
        ,m_pos(position)
        ,m_speed(0, 0)
        ,m_rotation(0)
        ,m_startTime(startTime)
        ,m_timeSlice(spriteData.lifeTime/spriteData.fragments.size())
        ,m_lastUpdated(QDateTime::currentMSecsSinceEpoch())
        ,m_repetitions(repetitions)
		,m_singleframe(single)
    	,m_kill_instantly(false)
    {
    }

    //square per millisec
    Sprite(const SpriteData& spriteData,
           const QPointF& position,
           const QPointF& speed,
           qreal rotation,
           qint64 startTime,
           int repetitions) //0 repetitions means forever
        :m_spriteData(&spriteData)
        ,m_currentFragment(0)
        ,m_pos(position)
        ,m_speed(speed)
        ,m_rotation(rotation)
        ,m_startTime(startTime)
        ,m_timeSlice(spriteData.lifeTime/spriteData.fragments.size())
        ,m_lastUpdated(QDateTime::currentMSecsSinceEpoch())
        ,m_repetitions(repetitions)
    	,m_singleframe(false)
    	,m_kill_instantly(false)
    {
    }



    const QPixmap& Image() const {return m_spriteData->image;}
    const QPointF& Position() const {return m_pos;}
    const QRectF& Fragment() const {return m_spriteData->fragments[m_currentFragment];}
    const qreal Rotation() const {return m_rotation;}
    const int Repetitions() const {return m_repetitions;}
    bool Started() const {return QDateTime::currentMSecsSinceEpoch()>=m_startTime;}
    bool Finished() const
    {
        if (m_repetitions==0){
            return false;
        }else if(m_kill_instantly){
        	return false;
        }else{
            return QDateTime::currentMSecsSinceEpoch()>m_startTime+m_repetitions*m_spriteData->lifeTime;
        }
    }

    void killToggle(){
    	if(m_singleframe){
    		m_kill_instantly = true;
    	}
    }

    bool killInstantly() {return m_kill_instantly;}

    void Update()
    {
        qreal now=QDateTime::currentMSecsSinceEpoch();
        if (Started())
        {
            m_currentFragment=(now-m_startTime)/m_timeSlice;

            if (m_repetitions==0 || m_currentFragment<m_spriteData->fragments.size()*m_repetitions)
            {
                m_currentFragment=m_currentFragment%m_spriteData->fragments.size();
            }
            else
            {
                m_currentFragment=m_spriteData->fragments.size()-1;
            }

            qreal elapsed=static_cast<qreal>(now-m_lastUpdated);
            m_pos.setX(m_pos.x()+m_speed.x()*elapsed);
            m_pos.setY(m_pos.y()+m_speed.y()*elapsed);
        }

        m_lastUpdated=now;
    }

    const SpriteData* Data() const {return m_spriteData;}

private:
    const SpriteData* m_spriteData;
    size_t m_currentFragment;
    QPointF m_pos;
    QPointF m_speed;
    qreal m_rotation;
    qint64 m_startTime;
    qint64 m_timeSlice;
    qint64 m_lastUpdated;
    int m_repetitions;
    bool m_singleframe;
    bool m_kill_instantly;
};

#endif
