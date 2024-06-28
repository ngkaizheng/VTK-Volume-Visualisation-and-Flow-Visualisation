import vtk
import sys

class MyInteractorStyle(vtk.vtkInteractorStyleTrackballCamera):
    def __init__(self, volumeScalarOpacity):
        self.volumeScalarOpacity = volumeScalarOpacity
        self.AddObserver("KeyPressEvent", self.keyPressEvent)

    def keyPressEvent(self, obj, event):
        key = self.GetInteractor().GetKeySym()
        if key == "Up":
            self.modifyOpacityFunction(0.1)  # Increase opacity
        elif key == "Down":
            self.modifyOpacityFunction(-0.1)  # Decrease opacity
        return

    def modifyOpacityFunction(self, delta):
        nodes = self.volumeScalarOpacity.GetSize()
        for i in range(nodes):
            value = list(self.volumeScalarOpacity.GetNodeValue(i))
            value[1] = max(0.0, min(1.0, value[1] + delta))
            self.volumeScalarOpacity.SetNodeValue(i, tuple(value))
        self.GetInteractor().GetRenderWindow().Render()


def main():
    if len(sys.argv) < 2:
        print("Usage: ", sys.argv[0], " quarter")
        return

    # Create the renderer, the render window, and the interactor
    ren = vtk.vtkRenderer()
    renWin = vtk.vtkRenderWindow()
    renWin.AddRenderer(ren)
    iren = vtk.vtkRenderWindowInteractor()
    iren.SetRenderWindow(renWin)

    # The following reader is used to read a series of 2D slices (images)
    # that compose the volume.
    v16 = vtk.vtkVolume16Reader()
    v16.SetDataDimensions(64, 64)
    v16.SetImageRange(1, 93)
    v16.SetDataByteOrderToLittleEndian()
    v16.SetFilePrefix(sys.argv[1])
    v16.SetDataSpacing(3.2, 3.2, 1.5)

    # The volume will be displayed by ray-cast alpha compositing.
    # A ray-cast mapper is needed to do the ray-casting.
    volumeMapper = vtk.vtkFixedPointVolumeRayCastMapper()
    volumeMapper.SetInputConnection(v16.GetOutputPort())

    # The color transfer function maps voxel intensities to colors.
    # It is modality-specific, and often anatomy-specific as well.
    # The goal is to one color for flesh (between 500 and 1000)
    # and another color for bone (1150 and over).
    volumeColor = vtk.vtkColorTransferFunction()
    volumeColor.AddRGBPoint(0, 0.0, 0.0, 0.0)
    volumeColor.AddRGBPoint(500, 1.0, 0.5, 0.3)
    volumeColor.AddRGBPoint(1000, 1.0, 0.5, 0.3)
    volumeColor.AddRGBPoint(1150, 1.0, 1.0, 0.9)

    # The opacity transfer function is used to control the opacity
    # of different tissue types.
    volumeScalarOpacity = vtk.vtkPiecewiseFunction()
    volumeScalarOpacity.AddPoint(0, 0.00)
    volumeScalarOpacity.AddPoint(500, 0.15)
    volumeScalarOpacity.AddPoint(1000, 0.15)
    volumeScalarOpacity.AddPoint(1150, 0.85)

    # The gradient opacity function is used to decrease the opacity
    # in the "flat" regions of the volume while maintaining the opacity
    # at the boundaries between tissue types.
    volumeGradientOpacity = vtk.vtkPiecewiseFunction()
    volumeGradientOpacity.AddPoint(0, 0.0)
    volumeGradientOpacity.AddPoint(90, 0.5)
    volumeGradientOpacity.AddPoint(100, 1.0)

    # The VolumeProperty attaches the color and opacity functions to the volume
    volumeProperty = vtk.vtkVolumeProperty()
    volumeProperty.SetColor(volumeColor)
    volumeProperty.SetScalarOpacity(volumeScalarOpacity)
    volumeProperty.SetGradientOpacity(volumeGradientOpacity)
    volumeProperty.SetInterpolationTypeToLinear()
    volumeProperty.ShadeOn()
    volumeProperty.SetAmbient(0.4)
    volumeProperty.SetDiffuse(0.6)
    volumeProperty.SetSpecular(0.2)

    # The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
    # and orientation of the volume in world coordinates.
    volume = vtk.vtkVolume()
    volume.SetMapper(volumeMapper)
    volume.SetProperty(volumeProperty)

    # Finally, add the volume to the renderer and set up the camera
    ren.AddVolume(volume)
    ren.ResetCamera()

    # Set up an initial view of the volume
    camera = ren.GetActiveCamera()
    c = volume.GetCenter()
    camera.SetFocalPoint(c[0], c[1], c[2])
    camera.SetPosition(c[0] + 400, c[1], c[2])
    camera.SetViewUp(0, 0, -1)

    # Increase the size of the render window
    renWin.SetSize(640, 480)

    # Set up interactor style to modify opacity
    style = MyInteractorStyle(volumeScalarOpacity)
    iren.SetInteractorStyle(style)

    # Interact with the data
    iren.Initialize()
    iren.Start()

if __name__ == "__main__":
    main()
