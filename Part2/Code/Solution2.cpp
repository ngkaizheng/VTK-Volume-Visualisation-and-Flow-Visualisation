#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStructuredPointsReader.h"
#include "vtkStructuredPoints.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkGlyph3D.h"
#include "vtkConeSource.h"
#include "vtkLookupTable.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkSliderWidget.h"
#include "vtkSliderRepresentation2D.h"
#include "vtkCommand.h"
#include "vtkAutoInit.h"
#include "vtkSmartPointer.h"

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle);

class SliderCallbackRadius : public vtkCommand {
public:
    static SliderCallbackRadius* New() {
        return new SliderCallbackRadius;
    }

    void Execute(vtkObject* caller, unsigned long, void*) override {
        vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
        double value = static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())->GetValue();
        this->ConeSource->SetRadius(value);
    }

    void SetConeSource(vtkConeSource* coneSource) {
        this->ConeSource = coneSource;
    }

private:
    vtkConeSource* ConeSource;
};

class SliderCallbackHeight : public vtkCommand {
public:
    static SliderCallbackHeight* New() {
        return new SliderCallbackHeight;
    }

    void Execute(vtkObject* caller, unsigned long, void*) override {
        vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
        double value = static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())->GetValue();
        this->ConeSource->SetHeight(value);
    }

    void SetConeSource(vtkConeSource* coneSource) {
        this->ConeSource = coneSource;
    }

private:
    vtkConeSource* ConeSource;
};

double determineScaleFactor(vtkStructuredPointsReader* reader) {
    vtkStructuredPoints* output = reader->GetOutput();
    if (!output) {
        return 1.0; // Default scale factor if no output is available
    }

    vtkDataArray* vectors = output->GetPointData()->GetVectors();
    if (!vectors) {
        return 1.0; // Default scale factor if no vectors are found
    }

    double maxMagnitude = 0.0;
    for (vtkIdType i = 0; i < vectors->GetNumberOfTuples(); ++i) {
        double vec[3];
        vectors->GetTuple(i, vec);
        double magnitude = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
        }
    }

    return 10.0 / maxMagnitude; // Adjust this constant as needed for visualization
}

int main(int argc, char** argv) {
    const char* filenames[] = {
        "../data/testData1.vtk",
        "../data/testData2.vtk",
        "../data/carotid.vtk",
    };

    for (int i = 0; i < 3; ++i) {
        vtkSmartPointer<vtkRenderer> aRenderer = vtkSmartPointer<vtkRenderer>::New();
        vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
        renWin->AddRenderer(aRenderer);
        vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        iren->SetRenderWindow(renWin);

        vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
        reader->SetFileName(filenames[i]);
        reader->Update();

        double scaleFactor = determineScaleFactor(reader);

        vtkSmartPointer<vtkConeSource> coneSource = vtkSmartPointer<vtkConeSource>::New();
        coneSource->SetRadius(0.1);
        coneSource->SetHeight(0.5);
        coneSource->SetResolution(10);

        vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
        glyph->SetInputConnection(reader->GetOutputPort());
        glyph->SetSourceConnection(coneSource->GetOutputPort());
        glyph->SetScaleFactor(scaleFactor);
        glyph->SetScaleModeToScaleByVector();
        glyph->OrientOn();
        glyph->Update();

        vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
        lut->SetHueRange(0.667, 0.0);
        lut->Build();

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(glyph->GetOutputPort());
        mapper->SetScalarRange(0.0, 1.0);
        mapper->SetLookupTable(lut);

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        aRenderer->AddActor(actor);
        aRenderer->SetBackground(0, 0, 0);
        renWin->SetSize(800, 600);

        // Create slider for radius
        vtkSmartPointer<vtkSliderRepresentation2D> sliderRepRadius = vtkSmartPointer<vtkSliderRepresentation2D>::New();
        sliderRepRadius->SetMinimumValue(0.05);
        sliderRepRadius->SetMaximumValue(0.5);
        sliderRepRadius->SetValue(coneSource->GetRadius());
        sliderRepRadius->SetTitleText("Radius");
        sliderRepRadius->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRepRadius->GetPoint1Coordinate()->SetValue(0.18, 0.1);
        sliderRepRadius->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRepRadius->GetPoint2Coordinate()->SetValue(0.48, 0.1);
        sliderRepRadius->SetSliderLength(0.02);
        sliderRepRadius->SetSliderWidth(0.03);
        sliderRepRadius->SetEndCapLength(0.02);

        vtkSmartPointer<vtkSliderWidget> sliderWidgetRadius = vtkSmartPointer<vtkSliderWidget>::New();
        sliderWidgetRadius->SetInteractor(iren);
        sliderWidgetRadius->SetRepresentation(sliderRepRadius);
        sliderWidgetRadius->SetAnimationModeToAnimate();
        sliderWidgetRadius->EnabledOn();

        vtkSmartPointer<SliderCallbackRadius> sliderCallbackRadius = vtkSmartPointer<SliderCallbackRadius>::New();
        sliderCallbackRadius->SetConeSource(coneSource);
        sliderWidgetRadius->AddObserver(vtkCommand::InteractionEvent, sliderCallbackRadius);

        // Create slider for height
        vtkSmartPointer<vtkSliderRepresentation2D> sliderRepHeight = vtkSmartPointer<vtkSliderRepresentation2D>::New();
        sliderRepHeight->SetMinimumValue(0.1);
        sliderRepHeight->SetMaximumValue(2.0);
        sliderRepHeight->SetValue(coneSource->GetHeight());
        sliderRepHeight->SetTitleText("Height");
        sliderRepHeight->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRepHeight->GetPoint1Coordinate()->SetValue(0.52, 0.1);
        sliderRepHeight->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
        sliderRepHeight->GetPoint2Coordinate()->SetValue(0.82, 0.1);
        sliderRepHeight->SetSliderLength(0.02);
        sliderRepHeight->SetSliderWidth(0.03);
        sliderRepHeight->SetEndCapLength(0.02);

        vtkSmartPointer<vtkSliderWidget> sliderWidgetHeight = vtkSmartPointer<vtkSliderWidget>::New();
        sliderWidgetHeight->SetInteractor(iren);
        sliderWidgetHeight->SetRepresentation(sliderRepHeight);
        sliderWidgetHeight->SetAnimationModeToAnimate();
        sliderWidgetHeight->EnabledOn();

        vtkSmartPointer<SliderCallbackHeight> sliderCallbackHeight = vtkSmartPointer<SliderCallbackHeight>::New();
        sliderCallbackHeight->SetConeSource(coneSource);
        sliderWidgetHeight->AddObserver(vtkCommand::InteractionEvent, sliderCallbackHeight);

        iren->Initialize();
        renWin->SetWindowName("Glyph-Based Volume Renderer with Sliders");
        renWin->Render();
        iren->Start();

        // Clean up
        reader->Delete();
        coneSource->Delete();
        glyph->Delete();
        lut->Delete();
        mapper->Delete();
        actor->Delete();
        iren->Delete();
        renWin->Delete();
        aRenderer->Delete();
    }

    return 0;
}