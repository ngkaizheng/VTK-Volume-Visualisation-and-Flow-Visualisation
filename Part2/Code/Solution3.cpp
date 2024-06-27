#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStructuredPointsReader.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkStreamTracer.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkGlyph3D.h"
#include "vtkArrowSource.h"
#include "vtkSliderWidget.h"
#include "vtkSliderRepresentation2D.h"
#include "vtkCommand.h"
#include "vtkAutoInit.h"
#include "vtkSmartPointer.h"

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle);

class SliderCallback : public vtkCommand {
public:
    static SliderCallback* New() {
        return new SliderCallback;
    }

    void Execute(vtkObject* caller, unsigned long, void*) override {
        vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
        double value = static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())->GetValue();
        this->IncrementValue = value;
        this->SetStartingPoints();
        this->RenderWindow->Render();
    }

    void SetRenderWindow(vtkRenderWindow* renderWindow) {
        this->RenderWindow = renderWindow;
    }

    void SetPolyData(vtkPolyData* polyData) {
        this->PolyData = polyData;
    }

    void SetReader(vtkStructuredPointsReader* reader) {
        this->Reader = reader;
    }

    void SetStartingPoints() {
        vtkPoints* points = vtkPoints::New();

if (strcmp(this->Dataset, "testData1") == 0) {
            for (int x = 0; x <= 36; x += this->IncrementValue) {
                for (int y = 0; y <= 36; y += this->IncrementValue) {
                    points->InsertNextPoint(x, y, 0);
                }
            }
        }
        else if (strcmp(this->Dataset, "testData2") == 0) {
            for (int x = 0; x <= 357; x += this->IncrementValue) {
                for (int y = 0; y <= 357; y += this->IncrementValue) {
                    points->InsertNextPoint(x, y, 0);
                }
            }
        }

        this->PolyData->SetPoints(points);
        points->Delete();
    }

    void SetDataset(const char* dataset) {
        this->Dataset = dataset;
    }

    void SetIncrementValue(int incrementValue) {
        this->IncrementValue = incrementValue;
    }

private:
    vtkRenderWindow* RenderWindow;
    vtkPolyData* PolyData;
    vtkStructuredPointsReader* Reader;
    const char* Dataset;
    int IncrementValue = 10; // Default value
};

int main(int argc, char** argv) {
    const char* filenames[] = {
        "../data/testData2.vtk",
    };
    const char* datasetNames[] = {
        "testData2",
    };
    for (int i = 0; i < 2; ++i) {
        vtkSmartPointer<vtkRenderer> aRenderer = vtkSmartPointer<vtkRenderer>::New();
        vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
        renWin->AddRenderer(aRenderer);
        vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        iren->SetRenderWindow(renWin);

        vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
        reader->SetFileName(filenames[i]);
        reader->Update();

        vtkSmartPointer<vtkPolyData> pointSet = vtkSmartPointer<vtkPolyData>::New();

        vtkSmartPointer<SliderCallback> sliderCallback = vtkSmartPointer<SliderCallback>::New();
        sliderCallback->SetRenderWindow(renWin);
        sliderCallback->SetPolyData(pointSet);
        sliderCallback->SetReader(reader);
        sliderCallback->SetDataset(datasetNames[i]);
        sliderCallback->SetStartingPoints();

        // Streamlines using vtkStreamTracer
        vtkSmartPointer<vtkStreamTracer> streamTracer = vtkSmartPointer<vtkStreamTracer>::New();
        streamTracer->SetInputConnection(reader->GetOutputPort());
        streamTracer->SetSourceData(pointSet);
        streamTracer->SetIntegrationDirectionToForward();
        streamTracer->SetMaximumPropagation(100.0);
        streamTracer->SetInitialIntegrationStep(0.1);
        streamTracer->SetIntegratorTypeToRungeKutta4();
        streamTracer->Update();

        vtkSmartPointer<vtkPolyDataMapper> streamMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        streamMapper->SetInputConnection(streamTracer->GetOutputPort());
        streamMapper->SetScalarRange(0.0, 1.0);

        vtkSmartPointer<vtkActor> streamActor = vtkSmartPointer<vtkActor>::New();
        streamActor->SetMapper(streamMapper);

        aRenderer->AddActor(streamActor);
        aRenderer->SetBackground(0, 0, 0);
        renWin->SetSize(800, 600);

        // Create slider for increment value
        vtkSmartPointer<vtkSliderRepresentation2D> sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
        sliderRep->SetMinimumValue(1);
        sliderRep->SetMaximumValue(20);
        sliderRep->SetValue(3); // Default increment value
        sliderRep->SetTitleText("Spacing");
        sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRep->GetPoint1Coordinate()->SetValue(0.35, 0.1);
        sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRep->GetPoint2Coordinate()->SetValue(0.65, 0.1);
        sliderRep->SetSliderLength(0.02);
        sliderRep->SetSliderWidth(0.03);
        sliderRep->SetEndCapLength(0.02);

        vtkSmartPointer<vtkSliderWidget> sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
        sliderWidget->SetInteractor(iren);
        sliderWidget->SetRepresentation(sliderRep);
        sliderWidget->SetAnimationModeToAnimate();
        sliderWidget->EnabledOn();

        sliderWidget->AddObserver(vtkCommand::InteractionEvent, sliderCallback);

        iren->Initialize();
        renWin->SetWindowName("Streamline and Glyph Visualization with Sliders");
        renWin->Render();
        iren->Start();

        // Clean up
        reader->Delete();
        pointSet->Delete();
        streamTracer->Delete();
        streamMapper->Delete();
        streamActor->Delete();
        iren->Delete();
        renWin->Delete();
        aRenderer->Delete();
    }

    return 0;
}
