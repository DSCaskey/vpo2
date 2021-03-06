/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef VISTOOLPIN_H
#define VISTOOLPIN_H

#include "visline.h"

class VSimplePoint;

class VisToolPin : public VisLine
{
    Q_OBJECT
public:
    explicit VisToolPin(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual ~VisToolPin();

    virtual void RefreshGeometry() Q_DECL_OVERRIDE;
    virtual int  type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::ToolPin)};
private:
    Q_DISABLE_COPY(VisToolPin)
    QPointer<VSimplePoint> m_point;
};

#endif // VISTOOLPIN_H
