#include "viewoptions.h"

ViewOptions::ViewOptions()
    : Image(NDTR_ORIGINAL),
      Shape(NDTR_CONTOUR),
      Fill(true),
      FillColor(0, 255, 0),
      Edge(true),
      EdgeColor(0, 0, 255),
      Center(true),
      CenterColor(255, 0, 0)
{

}
