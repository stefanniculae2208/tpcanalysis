import pandas as pd
import numpy as np
from skimage.measure import LineModelND, ransac
import matplotlib.pyplot as plt
from dataclasses import dataclass


@dataclass
class modelDataUVW:
    model_container: LineModelND = None
    min_val: float = None
    max_val: float = None
    slope: float = None
    intercept: float = None
    plane: int = None

    def calculateSlopeIntercept(self):
        self.slope = self.model_container.params[1][1] / \
            self.model_container.params[1][0]
        self.intercept = self.model_container.params[0][1] - \
            self.slope * self.model_container.params[0][0]


class processUVWLines:
    def __init__(self, entry):
        self._m_entry = entry
        self._m_hits = []

        # The index of the array will be equivalent to the value of the plane.
        for i in range(3):
            temp_entry = entry[entry['plane'] == i]
            self._m_hits.append(temp_entry[['time_bin', 'strip']].to_numpy())

        # The notation for each plane.
        self._m_plane_notations = {
            0: "U",
            1: "V",
            2: "W"
        }

        self._m_model_list = []

    def setEntry(self, entry):
        self._m_entry = entry

        for i in range(3):
            temp_entry = entry[entry['plane'] == i]
            self._m_hits.append(temp_entry[['time_bin', 'strip']].to_numpy())

    def drawHits(self, axes):

        for i in range(3):
            axes[i].scatter(self._m_hits[i][:, 0], self._m_hits[i][:, 1])
            axes[i].set_title('Plane ' + self._m_plane_notations)
            axes[i].set_xlim(-10, 520)
            axes[i].set_ylim(-10, 100)

    def _fitLinesForPlane(self, features, plane, axes,  n, enable_scatter=True, min_samples=2, residual_threshold=3, max_trials=1000):

        axes[plane].set_title('Plane ' + self._m_plane_notations[plane])
        axes[plane].set_xlim(-10, 520)
        axes[plane].set_ylim(-10, 100)
        axes[plane].grid(True)

        for i in range(n):

            if (len(features) > min_samples):

                model_var = modelDataUVW()

                model_var.plane = plane

                model, inliers = ransac(features, LineModelND, min_samples=min_samples,
                                        residual_threshold=residual_threshold, max_trials=max_trials, random_state=0)

                model_var.model_container = model

                outliers = inliers == False

                if (enable_scatter == True):
                    axes[plane].scatter(features[inliers][:, 0], features[inliers][:, 1],
                                        c='b', marker='o', label=f'Inliers {i+1}', alpha=0.2)
                    axes[plane].scatter(features[outliers][:, 0], features[outliers][:, 1],
                                        c='r', marker='o', label=f'Outliers {i+1}', alpha=0.2)

                line_points = np.linspace(features[inliers].min(
                    axis=0), features[inliers].max(axis=0), num=len(inliers))
                xy_line = model.predict(line_points[:, 0])
                axes[plane].plot(xy_line[:, 0], xy_line[:, 1],
                                 label=f'Fitted line {i+1}')

                # Save the minimum and maximum values on x for each model.

                model_var.min_val = features[inliers][:, 0].min()
                model_var.max_val = features[inliers][:, 0].max()

                # Calculates slope and intercept based on the model.
                model_var.calculateSlopeIntercept()

                self._m_model_list.append(model_var)

                # Update features to outliers for the next iteration
                features = features[outliers]
            else:
                print("Couldn't fit any more lines for plane " + self._m_plane_notations[plane] +
                      " at i = ", i)

        axes[plane].legend(loc='lower left')

    def fitNLines(self, axes,  n, enable_scatter=True, min_samples=2, residual_threshold=3, max_trials=1000):

        self._m_model_list.clear()

        for i in range(3):
            self._fitLinesForPlane(
                self._m_hits[i], i, axes, n, enable_scatter, min_samples, residual_threshold, max_trials)

        return self._m_model_list
