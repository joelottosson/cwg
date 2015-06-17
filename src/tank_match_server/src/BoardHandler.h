/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#pragma once

#include <iostream>
#include <vector>
#include <boost/make_shared.hpp>
#include <Safir/Dob/Connection.h>
#include <Safir/Dob/SuccessResponse.h>
#include <Safir/Dob/ErrorResponse.h>
#include <Safir/Dob/ResponseGeneralErrorCodes.h>

struct Point
{
    double x;
    double y;

    Point() : x(-1), y(-1) {} //default is an invalid state
    Point(double x_, double y_) : x(x_), y(y_) {}
};

typedef std::vector<Point> PointVec;

class BoardHandler
        :public Safir::Dob::EntityHandler
{
public:
    BoardHandler(Safir::Dob::Connection& con);
    void Init();

    //Returns true if file could be parsed without errors, if file='<generate_random>' then a
    //random board is generated.
    static bool FromFile(const std::string& file,
                         int& xSize,
                         int& ySize,
                         std::vector<char>& board,
                         Point& tank1,
                         Point& tank2,
						 Point& dude);

    static std::string GenerateRandomFile();

private:
    Safir::Dob::Connection& m_connection;
    Safir::Dob::Typesystem::HandlerId m_defaultHandler;
    Safir::Dob::Typesystem::InstanceId m_instance;

    void Refresh();
    bool ValidFile(const std::string& file) const;

    static std::string CreateFileName();

    // EntityHandler interface
    void OnCreateRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender);
    void OnUpdateRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender);
    void OnDeleteRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender);
    void OnRevokedRegistration(const Safir::Dob::Typesystem::TypeId typeId, const Safir::Dob::Typesystem::HandlerId &handlerId);
};
