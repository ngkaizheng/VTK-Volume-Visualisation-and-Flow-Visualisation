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
#include "vtkAutoInit.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkPolyLine.h"
#include "vtkStreamTracer.h"
#include "vtkSmartPointer.h"

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle);

void setStartingPoints(vtkPolyData* polyData, const char* dataset) {
    vtkPoints* points = vtkPoints::New();

    if (strcmp(dataset, "carotid") == 0) {
        for (int x = 0; x <= 76; x += 10) {
            for (int y = 0; y <= 49; y += 10) {
                for (int z = 0; z <= 45; z += 10) {
                    points->InsertNextPoint(x, y, z);
                }
            }
        }
    }
    else if (strcmp(dataset, "testData1") == 0) {
        for (int x = 0; x <= 36; x += 5) {
            for (int y = 0; y <= 36; y += 5) {
                points->InsertNextPoint(x, y, 0);
            }
        }
    }
    else if (strcmp(dataset, "testData2") == 0) {
        for (int x = 0; x <= 357; x += 20) {
            for (int y = 0; y <= 357; y += 20) {
                points->InsertNextPoint(x, y, 0);
            }
        }
    }

    polyData->SetPoints(points);
    points->Delete();
}

vtkSmartPointer<vtkPolyData> SampleStreamlinePoints(vtkSmartPointer<vtkPolyData> streamlines) {
    vtkSmartPointer<vtkPolyData> sampledPoints = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

    for (vtkIdType i = 0; i < streamlines->GetNumberOfCells(); ++i) {
        vtkCell* cell = streamlines->GetCell(i);
        vtkPolyLine* polyLine = vtkPolyLine::SafeDownCast(cell);

        if (polyLine) {
            vtkIdList* pointIds = polyLine->GetPointIds();
            for (vtkIdType j = 0; j < pointIds->GetNumberOfIds(); ++j) {
                double point[3];
                streamlines->GetPoint(pointIds->GetId(j), point);
                points->InsertNextPoint(point);
            }
        }
    }

    sampledPoints->SetPoints(points);
    return sampledPoints;
}

int main(int argc, char** argv) {
    const char* filenames[] = {
        "../data/testData1.vtk",
        "../data/testData2.vtk",
        "../data/carotid.vtk"
    };
    const char* datasetNames[] = {
        "testData1",
        "testData2",
        "carotid"
    };

    for (int i = 0; i < 3; ++i) {
        vtkRenderer* aRenderer = vtkRenderer::New();
        vtkRenderWindow* renWin = vtkRenderWindow::New();
        renWin->AddRenderer(aRenderer);
        vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
        iren->SetRenderWindow(renWin);

        vtkStructuredPointsReader* reader = vtkStructuredPointsReader::New();
        reader->SetFileName(filenames[i]);
        reader->Update();

        vtkPolyData* pointSet = vtkPolyData::New();
        setStartingPoints(pointSet, datasetNames[i]);

        vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New();

        // Streamlines using vtkStreamTracer
        vtkStreamTracer* streamTracer = vtkStreamTracer::New();
        streamTracer->SetInputConnection(reader->GetOutputPort());
        streamTracer->SetSourceData(pointSet);
        streamTracer->SetIntegrationDirectionToForward();
        streamTracer->SetMaximumPropagation(100.0);
        streamTracer->SetInitialIntegrationStep(0.1);
        streamTracer->SetIntegratorTypeToRungeKutta4();
        streamTracer->Update();

        vtkSmartPointer<vtkPolyData> sampledPoints = SampleStreamlinePoints(streamTracer->GetOutput());

        vtkPolyDataMapper* streamMapper = vtkPolyDataMapper::New();
        streamMapper->SetInputData(sampledPoints);
        streamMapper->SetScalarRange(0.0, 1.0);

        vtkActor* streamActor = vtkActor::New();
        streamActor->SetMapper(streamMapper);

        vtkGlyph3D* glyph = vtkGlyph3D::New();
        glyph->SetInputData(sampledPoints);
        glyph->SetInputConnection(reader->GetOutputPort());
        glyph->SetSourceConnection(arrowSource->GetOutputPort());
        glyph->SetVectorModeToUseVector();
        glyph->SetColorModeToColorByVector();

        if (i == 2) {
            glyph->SetScaleFactor(0.005);
        }
        else {
            glyph->SetScaleFactor(3.0); 
        }

        vtkPolyDataMapper* glyphMapper = vtkPolyDataMapper::New();
        glyphMapper->SetInputConnection(glyph->GetOutputPort());

        vtkActor* glyphActor = vtkActor::New();
        glyphActor->SetMapper(glyphMapper);

        aRenderer->AddActor(glyphActor);
        aRenderer->SetBackground(0, 0, 0);
        renWin->SetSize(800, 600);

        iren->Initialize();
        renWin->SetWindowName("Streamline and Glyph Visualization");
        renWin->Render();
        iren->Start();

        // Clean up
        reader->Delete();
        pointSet->Delete();
        streamTracer->Delete();
        glyph->Delete();
        arrowSource->Delete();
        glyphMapper->Delete();
        glyphActor->Delete();
        iren->Delete();
        renWin->Delete();
        aRenderer->Delete();
    }

    return 0;
}
