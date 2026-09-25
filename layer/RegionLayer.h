/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2008 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef SV_REGION_LAYER_H
#define SV_REGION_LAYER_H

#include "SingleColourLayer.h"
#include "ColourScaleLayer.h"

#include "data/model/RegionModel.h"
#include "base/ZoomLevel.h"

#include <QObject>
#include <QColor>

#include <map>
#include <set>
#include <vector>

class QPainter;

namespace sv {

class View;

class RegionLayer : public SingleColourLayer,
                    public ColourScaleLayer
{
    Q_OBJECT

public:
    RegionLayer();

    void paint(LayerGeometryProvider *v, QPainter &paint, QRect rect) const override;

    int getVerticalScaleWidth(LayerGeometryProvider *v, bool, QPainter &) const override;
    void paintVerticalScale(LayerGeometryProvider *v, bool, QPainter &paint, QRect rect) const override;

    QString getFeatureDescription(LayerGeometryProvider *v, QPoint &) const override;
    QString getLabelAtOrPreceding(sv_frame_t) const override;

    bool snapToFeatureFrame(LayerGeometryProvider *v, sv_frame_t &frame,
                            int &resolution,
                            SnapType snap, int ycoord) const override;
    bool snapToSimilarFeature(LayerGeometryProvider *v, sv_frame_t &frame,
                              int &resolution,
                              SnapType snap) const override;

    void drawStart(LayerGeometryProvider *v, QMouseEvent *) override;
    void drawDrag(LayerGeometryProvider *v, QMouseEvent *) override;
    void drawEnd(LayerGeometryProvider *v, QMouseEvent *) override;

    void eraseStart(LayerGeometryProvider *v, QMouseEvent *) override;
    void eraseDrag(LayerGeometryProvider *v, QMouseEvent *) override;
    void eraseEnd(LayerGeometryProvider *v, QMouseEvent *) override;

    void editStart(LayerGeometryProvider *v, QMouseEvent *) override;
    void editDrag(LayerGeometryProvider *v, QMouseEvent *) override;
    void editEnd(LayerGeometryProvider *v, QMouseEvent *) override;

    bool editOpen(LayerGeometryProvider *v, QMouseEvent *) override;

    void moveSelection(Selection s, sv_frame_t newStartFrame) override;
    void resizeSelection(Selection s, Selection newSize) override;
    void deleteSelection(Selection s) override;

    void copy(LayerGeometryProvider *v, Selection s, Clipboard &to) override;
    bool paste(LayerGeometryProvider *v, const Clipboard &from, sv_frame_t frameOffset,
                       bool interactive) override;

    ModelId getModel() const override { return m_model; }
    void setModel(ModelId model); // a RegionModel

    PropertyList getProperties() const override;
    QString getPropertyLabel(const PropertyName &) const override;
    PropertyType getPropertyType(const PropertyName &) const override;
    QString getPropertyGroupName(const PropertyName &) const override;
    int getPropertyRangeAndValue(const PropertyName &,
                                         int *min, int *max, int *deflt) const override;
    QString getPropertyValueLabel(const PropertyName &,
                                          int value) const override;
    void setProperty(const PropertyName &, int value) override;

    void setFillColourMap(int);
    int getFillColourMap() const { return m_colourMap; }

    enum VerticalScale {
        AutoAlignScale,
        EqualSpaced,
        LinearScale,
        LogScale,
    };

    void setVerticalScale(VerticalScale scale);
    VerticalScale getVerticalScale() const { return m_verticalScale; }

    enum PlotStyle {
        PlotLines,
        PlotSegmentation,

        // Each region a filled bar in a band of fixed height along the
        // bottom of the view, whatever its value: for showing where
        // there is something and where there is not.  Display only: no
        // vertical scale, no labels, no editing
        PlotStrip,

        // Each region's label written along the top of the view at the
        // region's start, over a thin bar as long as the region: for
        // words and when they are sung.  A label that would run into
        // the one before goes to a second row, and is left out when
        // that row has no room either.  Display only, as PlotStrip
        PlotLyrics
    };

    void setPlotStyle(PlotStyle style);
    PlotStyle getPlotStyle() const { return m_plotStyle; }

    bool isLayerScrollable(const LayerGeometryProvider *v) const override;

    bool isLayerEditable() const override {
        return m_plotStyle != PlotStrip && m_plotStyle != PlotLyrics;
    }

    /**
     * The rows of the labels of PlotLyrics.  Each label is given as
     * its left edge and width, in order of left edge.  A label goes
     * in the first of the given number of rows whose last label ends
     * at least gap before it, or gets -1 if there is no such row.
     */
    static std::vector<int> assignLabelRows
    (const std::vector<std::pair<double, double>> &xAndWidth,
     int rows, double gap);

    int getCompletion(LayerGeometryProvider *) const override;

    ScaleExtents getVerticalExtents() const override;

    bool getDisplayExtents(double &min, double &max) const override;

    void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const override;

    void setProperties(const LayerAttributes &attributes) override;

    /// ColourScaleLayer methods
    QString getScaleUnits() const override;
    QColor getColourForValue(LayerGeometryProvider *v, double value) const override;

protected slots:
    void recalcSpacing();

protected:
    double getValueForY(LayerGeometryProvider *v, int y, int avoid) const;
    double getValueForY(LayerGeometryProvider *v, int y) const;
    int getYForValue(LayerGeometryProvider *v, double value) const;

    int getDefaultColourHint(bool dark, bool &impose) override;

    EventVector getLocalPoints(LayerGeometryProvider *v, int x) const;

    bool getPointToDrag(LayerGeometryProvider *v, int x, int y, Event &) const;

    void paintLyrics(LayerGeometryProvider *v, QPainter &paint, QRect rect) const;

    ModelId m_model;
    bool m_editing;
    int m_dragPointX;
    int m_dragPointY;
    int m_dragStartX;
    int m_dragStartY;
    Event m_originalPoint;
    Event m_editingPoint;
    ChangeEventsCommand *m_editingCommand;
    VerticalScale m_verticalScale;
    int m_colourMap;
    bool m_colourInverted;
    PlotStyle m_plotStyle;
    bool m_propertiesExplicitlySet;

    typedef std::map<double, int> SpacingMap;

    // region value -> ordering
    SpacingMap m_spacingMap;

    // region value -> number of regions with this value
    SpacingMap m_distributionMap;

    // Where the labels of PlotLyrics go depends on the labels before
    // them, so they are laid out for the whole model at once, for one
    // zoom level and font, not in each paint: a view that scrolls
    // repaints only the part that comes into sight, and that has to
    // agree with what is on show already
    struct LyricsLayout {
        bool valid = false;
        ZoomLevel zoom;
        QString font;
        int eventCount = 0;
        sv_frame_t startFrame = 0;
        sv_frame_t endFrame = 0;
        std::map<Event, int> rows;       // -1 for a label left out
        std::set<Event> lineStarts;      // drawn in bold
        int maxWidth = 0;
    };
    mutable LyricsLayout m_lyricsLayout;

    int spacingIndexToY(LayerGeometryProvider *v, int i) const;
    double yToSpacingIndex(LayerGeometryProvider *v, int y) const;

    void finish(ChangeEventsCommand *command) {
        Command *c = command->finish();
        if (c) CommandHistory::getInstance()->addCommand(c, false);
    }
};

} // end namespace sv

#endif

