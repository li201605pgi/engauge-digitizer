#include "CurveConstants.h"
#include "CurveFilter.h"
#include "EngaugeAssert.h"
#include "Filter.h"
#include <QPainter>
#include <QPixmap>
#include "ViewSegmentFilter.h"

ViewSegmentFilter::ViewSegmentFilter(QWidget *parent) :
  QLabel (parent),
  m_filterIsDefined (false),
  m_rgbBackground (QColor (Qt::white))
{
  // Note the size is set externally by the layout engine
}

QColor ViewSegmentFilter::colorFromSetting (FilterMode filterMode,
                                            int foreground,
                                            int hue,
                                            int intensity,
                                            int saturation,
                                            int value) const
{
  int r = 0, g = 0, b = 0;

  switch (filterMode)
  {
    case FILTER_MODE_FOREGROUND:
      {
        double s = (double) (foreground - FOREGROUND_MIN) / (double) (FOREGROUND_MAX - FOREGROUND_MIN);
        if (qGray (m_rgbBackground.rgb ()) < 127) {
          // Go from blackish to white
          r = s * 255;
          g = s * 255;
          b = s * 255;
        } else {
          // Go from whitish to black
          r = (1.0 - s) * 255;
          g = (1.0 - s) * 255;
          b = (1.0 - s) * 255;
        }
      }
      break;

    case FILTER_MODE_HUE:
      {
        // red-green and green-blue like ViewProfileScale::paintHue

        int HUE_THRESHOLD_LOW = 0.666 * HUE_MIN + 0.333 * HUE_MAX;
        int HUE_THRESHOLD_HIGH = 0.333 * HUE_MIN + 0.666 * HUE_MAX;

        if (hue < HUE_THRESHOLD_LOW) {
          // 0-0.333 is red-green
          double s = (double) (hue - HUE_MIN) / (double) (HUE_THRESHOLD_LOW - HUE_MIN);
          r = (1.0 - s) * 255;
          g = s * 255;
        } else if (hue < HUE_THRESHOLD_HIGH) {
          // 0.333-0.666 is green-blue
          double s = (double) (hue - HUE_THRESHOLD_LOW) / (double) (HUE_THRESHOLD_HIGH - HUE_THRESHOLD_LOW);
          g = (1.0 - s) * 255;
          b = s * 255;
        } else {
          // 0.666-1 is blue-red
          double s = (double) (hue - HUE_THRESHOLD_HIGH) / (double) (HUE_MAX - HUE_THRESHOLD_HIGH);
          b = (1.0 - s) * 255;
          r = s * 255;
        }
      }
      break;

    case FILTER_MODE_INTENSITY:
      {
        // black-white like ViewProfileScale::paintIntensity

        double s = (double) (intensity - INTENSITY_MIN) / (double) (INTENSITY_MAX - INTENSITY_MIN);
        r = s * 255;
        g = s * 255;
        b = s * 255;
      }
      break;

    case FILTER_MODE_SATURATION:
      {
        // white-red like ViewProfileScale::paintSaturation

        double s = (double) (saturation - SATURATION_MIN) / (double) (SATURATION_MAX - SATURATION_MIN);
        r = 255;
        g = (1.0 - s) * 255;
        b = (1.0 - s) * 255;
      }
      break;

    case FILTER_MODE_VALUE:
      {
        // black-red like ViewProfileScale::paintValue

        double s = (double) (value - VALUE_MIN) / (double) (VALUE_MAX - VALUE_MIN);
        r = s * 255;
        g = 0;
        b = 0;
      }
      break;

    default:
      ENGAUGE_ASSERT (false);
  }

  return QColor (r, g, b);
}

QColor ViewSegmentFilter::colorHigh () const
{
  return colorFromSetting (m_curveFilter.filterMode (),
                           m_curveFilter.foregroundHigh (),
                           m_curveFilter.hueHigh (),
                           m_curveFilter.intensityHigh(),
                           m_curveFilter.saturationHigh(),
                           m_curveFilter.valueHigh());
}

QColor ViewSegmentFilter::colorLow () const
{
  return colorFromSetting (m_curveFilter.filterMode (),
                           m_curveFilter.foregroundLow (),
                           m_curveFilter.hueLow (),
                           m_curveFilter.intensityLow(),
                           m_curveFilter.saturationLow(),
                           m_curveFilter.valueLow());
}

void ViewSegmentFilter::paintEvent(QPaintEvent * /* event */)
{
  QPainter painter(this);

  if (m_filterIsDefined) {

    // Start and end points are midway up on both sides
    QLinearGradient gradient (0, height()/2, width(), height()/2);

    // One color at either end
    gradient.setColorAt (0.0, colorLow ());
    gradient.setColorAt (1.0, colorHigh ());
    painter.setBrush (gradient);

    // No border, which is consistent with ViewPointStyle and cleaner
    painter.setPen (Qt::NoPen);

    painter.drawRect (0, 0, width(), height());

  } else {

    painter.fillRect (0, 0, width (), height (), QBrush (Qt::white));

  }
}

void ViewSegmentFilter::setCurveFilter (const CurveFilter &curveFilter,
                                        const QPixmap &pixmap)
{
  m_curveFilter = curveFilter;
  m_filterIsDefined = true;

  // Compute background color
  Filter filter;
  QImage img = pixmap.toImage();
  m_rgbBackground = filter.marginColor(&img);

  // Force a redraw
  update();
}

void ViewSegmentFilter::unsetCurveFilter ()
{
  m_filterIsDefined = false;

  // Force a redraw
  update();
}
