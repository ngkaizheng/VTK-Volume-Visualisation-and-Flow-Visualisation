#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStructuredPointsReader.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkHedgeHog.h"
#include "vtkLookupTable.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkSliderRepresentation2D.h"
#include "vtkSliderWidget.h"
#include "vtkCallbackCommand.h"
#include "vtkAutoInit.h"
#include "vtkStructuredPoints.h"
#include "vtkSmartPointer.h"

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle);

class SliderCallback : public vtkCommand
{
public:
    static SliderCallback* New()
    {
        return new SliderCallback;
    }

    void Execute(vtkObject* caller, unsigned long, void*) override
    {
        vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
        double value = static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())->GetValue();
        this->ScaleFactor = value;
        this->UpdateHedgeHog();
    }

    void SetHedgeHog(vtkHedgeHog* hh)
    {
        this->HedgeHog = hh;
    }

    void SetScaleFactor(double scaleFactor)
    {
        this->ScaleFactor = scaleFactor;
    }

    void SetReader(vtkStructuredPointsReader* reader)
    {
        this->Reader = reader;
    }

    void UpdateHedgeHog()
    {
        double maxMagnitude = 0.0;
        vtkDataArray* vectors = this->Reader->GetOutput()->GetPointData()->GetVectors();
        for (vtkIdType i = 0; i < vectors->GetNumberOfTuples(); ++i)
        {
            double vec[3];
            vectors->GetTuple(i, vec);
            double magnitude = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
            if (magnitude > maxMagnitude)
            {
                maxMagnitude = magnitude;
            }
        }

        this->HedgeHog->SetScaleFactor(this->ScaleFactor / maxMagnitude);
        this->HedgeHog->Update();
    }

private:
    vtkHedgeHog* HedgeHog;
    vtkStructuredPointsReader* Reader;
    double ScaleFactor;
};

int main(int argc, char** argv)
{
    const char* filenames[] = {
        "../data/testData1.vtk",
        "../data/testData2.vtk",
        "../data/carotid.vtk" };

    for (int i = 0; i < 3; ++i)
    {
        vtkSmartPointer<vtkRenderer> aRenderer = vtkSmartPointer<vtkRenderer>::New();
        vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
        renWin->AddRenderer(aRenderer);
        vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        iren->SetRenderWindow(renWin);

        vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
        reader->SetFileName(filenames[i]);
        reader->Update();

        double initialScaleFactor = 3.0;

        vtkSmartPointer<vtkHedgeHog> hhog = vtkSmartPointer<vtkHedgeHog>::New();
        hhog->SetInputConnection(reader->GetOutputPort());

        vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
        lut->SetHueRange(0.667, 0.0);
        lut->Build();

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(hhog->GetOutputPort());
        mapper->SetScalarRange(0.0, 1.0);
        mapper->SetLookupTable(lut);

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        aRenderer->AddActor(actor);
        aRenderer->SetBackground(0, 0, 0);
        renWin->SetSize(800, 600);

        vtkSmartPointer<SliderCallback> sliderCallback = vtkSmartPointer<SliderCallback>::New();
        sliderCallback->SetHedgeHog(hhog);
        sliderCallback->SetReader(reader);
        sliderCallback->SetScaleFactor(initialScaleFactor);

        // Set up the slider
        vtkSmartPointer<vtkSliderRepresentation2D> sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
        sliderRep->SetMinimumValue(1.0); // Adjust this as needed
        sliderRep->SetMaximumValue(50.0); // Adjust this as needed
        sliderRep->SetValue(initialScaleFactor); // Initial value
        sliderRep->SetTitleText("Scale Factor");
        sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRep->GetPoint1Coordinate()->SetValue(0.3, 0.1);
        sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRep->GetPoint2Coordinate()->SetValue(0.7, 0.1);
        sliderRep->SetSliderLength(0.02);
        sliderRep->SetSliderWidth(0.03);
        sliderRep->SetEndCapLength(0.02);

        vtkSmartPointer<vtkSliderWidget> sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
        sliderWidget->SetInteractor(iren);
        sliderWidget->SetRepresentation(sliderRep);
        sliderWidget->SetAnimationModeToAnimate();
        sliderWidget->EnabledOn();
        sliderWidget->AddObserver(vtkCommand::InteractionEvent, sliderCallback);

        sliderCallback->UpdateHedgeHog(); // Initial update

        iren->Initialize();
        renWin->SetWindowName("Simple Volume Renderer with Slider");
        renWin->Render();
        iren->Start();
    }

    return 0;
}