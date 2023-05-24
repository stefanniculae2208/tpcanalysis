import pandas as pd
import numpy as np
from skimage.measure import LineModelND, ransac
from dataclasses import dataclass


@dataclass
class modelDataXYZ:
    model_container: LineModelND = None
    min_val: float = None
    max_val: float = None


class processXYZLines:

    def __init__(self, entry):

        self._m_entry = entry

        self._m_features = self._m_entry[['x', 'y', 'z']].to_numpy()

        self._m_model_list = []

    def setEntry(self, entry):

        self._m_entry = entry

        self._m_features = self._m_entry[['x', 'y', 'z']].to_numpy()

    def fitNLines(self, fig,  n, enable_scatter=True, min_samples=2, residual_threshold=3, max_trials=1000):

        self._m_model_list.clear()

        ax = fig.add_subplot(111, projection='3d')
        ax.set_xlim(-10, 150)
        ax.set_ylim(-10, 150)
        ax.set_zlim(-10, 150)

        features = self._m_features

        for i in range(n):

            if (len(features) > min_samples):

                model_var = modelDataXYZ()

                model_robust, inliers = ransac(features, LineModelND, min_samples=min_samples,
                                               residual_threshold=residual_threshold, max_trials=max_trials, random_state=0)

                model_var.model_container = model_robust

                outliers = inliers == False

                if (enable_scatter == True):
                    ax.scatter(features[inliers][:, 0], features[inliers][:, 1],
                               features[inliers][:, 2], c='b', marker='o', label=f'Inliers {i+1}', alpha=0.2)
                    ax.scatter(features[outliers][:, 0], features[outliers][:, 1],
                               features[outliers][:, 2], c='r', marker='o', label=f'Outliers {i+1}', alpha=0.2)

                line_points = np.linspace(features[inliers].min(
                    axis=0), features[inliers].max(axis=0), num=len(inliers))
                xyz_line = model_robust.predict(line_points[:, 0])
                ax.plot(xyz_line[:, 0], xyz_line[:, 1],
                        xyz_line[:, 2], label=f'Fitted line {i+1}')

                # Save the minimum and maximum values on x for each model

                model_var.min_val = features[inliers][:, 0].min()
                model_var.max_val = features[inliers][:, 0].max()

                self._m_model_list.append(model_var)

                # Update features to outliers for the next iteration
                features = features[outliers]
            else:
                print("Couldn't fit any more lines at i = ", i)

        ax.legend(loc='lower left')

        return self._m_model_list
