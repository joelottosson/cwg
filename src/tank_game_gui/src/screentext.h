/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#pragma once

#include <QTime>
#include <QStringList>
#include <QPen>

class ScreenText
{
public:

    ScreenText(const QStringList& multilineText,
               const QPointF& position, //-1 means allign center, (-1,-1)->CenterOfScreen, (-1, 0)->X_AllignCenter_ yAllignTop
               const QColor& color,
               int fontSize,
               int penWidth,
               const QPointF& speed, //squares per milli
               bool fade,
               qint64 duration) //milli
        :m_text(multilineText)
        ,m_pos(position)
        ,m_fontSize(fontSize)
        ,m_speed(speed)
        ,m_fade(fade)
        ,m_endTime(QDateTime::currentMSecsSinceEpoch()+duration)
        ,m_lastUpdated(QDateTime::currentMSecsSinceEpoch())
        ,m_pen(Qt::SolidLine)
        ,m_lastSetAlpha(color.alpha())
        ,m_floatingAlpha(static_cast<qreal>(m_lastSetAlpha))
        ,m_alphaDecreasePerMilli(m_floatingAlpha/static_cast<qreal>(duration))
    {
        m_pen.setWidth(penWidth);
        m_pen.setColor(color);
    }

    const QPointF& Position() const {return m_pos;}
    bool IsMultiline() const {return false;}
    const QStringList& Text() const {return m_text;}
    int FontSize() const {return m_fontSize;}
    const QPen& Pen() const {return m_pen;}

    bool Finished() const {return QDateTime::currentMSecsSinceEpoch()>m_endTime;}

    void Update()
    {
        qint64 now=QDateTime::currentMSecsSinceEpoch();
        if (!Finished())
        {
            qreal elapsed=static_cast<qreal>(now-m_lastUpdated);
            m_pos.setX(m_pos.x()+m_speed.x()*elapsed);
            m_pos.setY(m_pos.y()+m_speed.y()*elapsed);

            if (m_fade)
            {
                m_floatingAlpha-=(m_alphaDecreasePerMilli*elapsed);
                int newAlpha=std::max(static_cast<int>(m_floatingAlpha), 0);
                if (m_lastSetAlpha-newAlpha>5)
                {
                    auto color=m_pen.color();
                    color.setAlpha(newAlpha);
                    m_pen.setColor(color);
                    m_lastSetAlpha=newAlpha;
                }
            }
        }

        m_lastUpdated=now;
    }

private:
    QStringList m_text;
    QPointF m_pos;
    int m_fontSize;
    QPointF m_speed;
    bool m_fade;
    qint64 m_startTime;
    qint64 m_endTime;
    qint64 m_lastUpdated;
    QPen m_pen;
    int m_lastSetAlpha;
    qreal m_floatingAlpha;
    qreal m_alphaDecreasePerMilli;
};
