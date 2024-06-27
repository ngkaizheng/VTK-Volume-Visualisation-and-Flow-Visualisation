#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkContourFilter.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOutlineFilter.h>
#include <vtkPointData.h>
#include <vtkPointSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkStreamTracer.h>
#include <vtkStructuredPointsReader.h>
#include <vtkThresholdPoints.h>
#include <vtkTubeFilter.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>

#include <iostream>
#include <string>

// Callback for the slider to update the tube radius
class vtkSliderCallback : public vtkCommand
{
public:
    static vtkSliderCallback* New()
    {
        return new vtkSliderCallback;
    }

    vtkSliderCallback() : TubeFilter(nullptr), PointSource(nullptr) {}

    void Execute(vtkObject* caller, unsigned long, void*) override
    {
        vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
        double value = static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())->GetValue();

        if (sliderWidget == TubeRadiusSliderWidget)
        {
            this->TubeFilter->SetRadius(value);
            this->TubeFilter->Update();
        }
        else if (sliderWidget == NumberOfPointsSliderWidget)
        {
            int numPoints = static_cast<int>(value);
            this->PointSource->SetNumberOfPoints(numPoints);
            this->PointSource->Update();
        }
    }

    vtkTubeFilter* TubeFilter;
    vtkPointSource* PointSource;
    vtkSliderWidget* TubeRadiusSliderWidget;
    vtkSliderWidget* NumberOfPointsSliderWidget;
};

int main(int argc, char** argv)
{
    const char* filenames[] = {
        "../data/carotid.vtk"
    };

    vtkNew<vtkNamedColors> colors;

    vtkNew<vtkRenderer> ren1;

    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren1);

    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    // Create pipeline
    vtkNew<vtkStructuredPointsReader> reader;
    reader->SetFileName(filenames[0]);

    vtkNew<vtkPointSource> psource;
    psource->SetNumberOfPoints(25); // Initial number of points
    psource->SetCenter(133.1, 116.3, 5.0);
    psource->SetRadius(2.0);

    vtkNew<vtkThresholdPoints> threshold;
    threshold->SetInputConnection(reader->GetOutputPort());
    threshold->ThresholdByUpper(275);

    vtkNew<vtkStreamTracer> streamers;
    streamers->SetInputConnection(reader->GetOutputPort());
    streamers->SetSourceConnection(psource->GetOutputPort());
    streamers->SetMaximumPropagation(100.0);
    streamers->SetInitialIntegrationStep(0.2);
    streamers->SetTerminalSpeed(.01);
    streamers->Update();
    double range[2];
    range[0] = streamers->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
    range[1] = streamers->GetOutput()->GetPointData()->GetScalars()->GetRange()[1];

    vtkNew<vtkTubeFilter> tubes;
    tubes->SetInputConnection(streamers->GetOutputPort());
    tubes->SetRadius(0.3); // Initial radius value
    tubes->SetNumberOfSides(6);
    tubes->SetVaryRadius(0);

    vtkNew<vtkLookupTable> lut;
    lut->SetHueRange(.667, 0.0);
    lut->Build();

    vtkNew<vtkPolyDataMapper> streamerMapper;
    streamerMapper->SetInputConnection(tubes->GetOutputPort());
    streamerMapper->SetScalarRange(range[0], range[1]);
    streamerMapper->SetLookupTable(lut);

    vtkNew<vtkActor> streamerActor;
    streamerActor->SetMapper(streamerMapper);

    // Contours of speed
    vtkNew<vtkContourFilter> iso;
    iso->SetInputConnection(reader->GetOutputPort());
    iso->SetValue(0, 175);

    vtkNew<vtkPolyDataMapper> isoMapper;
    isoMapper->SetInputConnection(iso->GetOutputPort());
    isoMapper->ScalarVisibilityOff();

    vtkNew<vtkActor> isoActor;
    isoActor->SetMapper(isoMapper);
    isoActor->GetProperty()->SetRepresentationToWireframe();
    isoActor->GetProperty()->SetOpacity(0.25);

    // Outline
    vtkNew<vtkOutlineFilter> outline;
    outline->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkPolyDataMapper> outlineMapper;
    outlineMapper->SetInputConnection(outline->GetOutputPort());

    vtkNew<vtkActor> outlineActor;
    outlineActor->SetMapper(outlineMapper);
    outlineActor->GetProperty()->SetColor(colors->GetColor3d("White").GetData());

    // Add the actors to the renderer, set the background and size
    ren1->AddActor(outlineActor);
    ren1->AddActor(streamerActor);
    ren1->AddActor(isoActor);
    ren1->SetBackground(colors->GetColor3d("Black").GetData());
    renWin->SetSize(640, 480);
    renWin->SetWindowName("CarotidFlow");

    vtkNew<vtkCamera> cam1;
    cam1->SetClippingRange(17.4043, 870.216);
    cam1->SetFocalPoint(136.71, 104.025, 23);
    cam1->SetPosition(204.747, 258.939, 63.7925);
    cam1->SetViewUp(-0.102647, -0.210897, 0.972104);
    cam1->Zoom(1.2);
    ren1->SetActiveCamera(cam1);

    // Create the tube radius slider widget
    vtkNew<vtkSliderRepresentation2D> tubeRadiusSliderRep;
    tubeRadiusSliderRep->SetMinimumValue(0.1);
    tubeRadiusSliderRep->SetMaximumValue(1.0);
    tubeRadiusSliderRep->SetValue(0.3); // Initial value
    tubeRadiusSliderRep->SetTitleText("Tube Radius");
    tubeRadiusSliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    tubeRadiusSliderRep->GetPoint1Coordinate()->SetValue(0.1, 0.1);
    tubeRadiusSliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    tubeRadiusSliderRep->GetPoint2Coordinate()->SetValue(0.4, 0.1);
    tubeRadiusSliderRep->SetSliderLength(0.02);
    tubeRadiusSliderRep->SetSliderWidth(0.03);
    tubeRadiusSliderRep->SetEndCapLength(0.01);
    tubeRadiusSliderRep->SetEndCapWidth(0.02);
    tubeRadiusSliderRep->SetTubeWidth(0.005);
    tubeRadiusSliderRep->SetLabelFormat("%0.2f");
    tubeRadiusSliderRep->SetTitleHeight(0.02);
    tubeRadiusSliderRep->SetLabelHeight(0.02);

    vtkNew<vtkSliderWidget> tubeRadiusSliderWidget;
    tubeRadiusSliderWidget->SetInteractor(iren);
    tubeRadiusSliderWidget->SetRepresentation(tubeRadiusSliderRep);
    tubeRadiusSliderWidget->SetAnimationModeToAnimate();

    // Create the number of points slider widget
    vtkNew<vtkSliderRepresentation2D> numberOfPointsSliderRep;
    numberOfPointsSliderRep->SetMinimumValue(10);
    numberOfPointsSliderRep->SetMaximumValue(100);
    numberOfPointsSliderRep->SetValue(25); // Initial value
    numberOfPointsSliderRep->SetTitleText("Number of Points");
    numberOfPointsSliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    numberOfPointsSliderRep->GetPoint1Coordinate()->SetValue(0.1, 0.17);
    numberOfPointsSliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    numberOfPointsSliderRep->GetPoint2Coordinate()->SetValue(0.4, 0.17);
    numberOfPointsSliderRep->SetSliderLength(0.02);
    numberOfPointsSliderRep->SetSliderWidth(0.03);
    numberOfPointsSliderRep->SetEndCapLength(0.01);
    numberOfPointsSliderRep->SetEndCapWidth(0.02);
    numberOfPointsSliderRep->SetTubeWidth(0.005);
    numberOfPointsSliderRep->SetLabelFormat("%0.f");
    numberOfPointsSliderRep->SetTitleHeight(0.02);
    numberOfPointsSliderRep->SetLabelHeight(0.02);

    vtkNew<vtkSliderWidget> numberOfPointsSliderWidget;
    numberOfPointsSliderWidget->SetInteractor(iren);
    numberOfPointsSliderWidget->SetRepresentation(numberOfPointsSliderRep);
    numberOfPointsSliderWidget->SetAnimationModeToAnimate();

    // Create callback for sliders
    vtkNew<vtkSliderCallback> callback;
    callback->TubeFilter = tubes;
    callback->PointSource = psource;
    callback->TubeRadiusSliderWidget = tubeRadiusSliderWidget;
    callback->NumberOfPointsSliderWidget = numberOfPointsSliderWidget;

    tubeRadiusSliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);
    numberOfPointsSliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);

    // Render the image and start interaction
    renWin->Render();
    tubeRadiusSliderWidget->EnabledOn();
    numberOfPointsSliderWidget->EnabledOn();
    iren->Start();

    return EXIT_SUCCESS;
}
