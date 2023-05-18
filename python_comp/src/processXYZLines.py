import pandas as pd
import numpy as np
from skimage.measure import LineModelND, ransac


class processXYZLines:

    def __init__(self, entry):
        self._m_entry = entry
        self._m_features = self._m_entry[['x', 'y', 'z']].to_numpy()

    def setEntry(self, entry):
        self._m_entry = entry
        self._m_features = self._m_entry[['x', 'y', 'z']].to_numpy()

    def getLine(self):
        """ Returns the xyz_line, the features, the inliers and the outliers. """

        model_robust, inliers = ransac(self._m_features, LineModelND, min_samples=2,
                                       residual_threshold=3, max_trials=1000, random_state=0)
        outliers = inliers == False

        line_points = np.linspace(self._m_features[inliers].min(
            axis=0), self._m_features[inliers].max(axis=0), num=len(inliers))
        xyz_line = model_robust.predict(line_points[:, 0])

        return xyz_line, self._m_features, inliers, outliers

    def drawLine(self, fig, min_samples=2, residual_threshold=3, max_trials=1000):
        """ Needs a plt.figure() as input. After this function just plt.show() and it should work. """
        """ Can also receive the min_samples, residual_threshold and max_trials as arguments, which go directly in ransac. """

        model_robust, inliers = ransac(self._m_features, LineModelND, min_samples=min_samples,
                                       residual_threshold=residual_threshold, max_trials=max_trials, random_state=0)
        outliers = inliers == False

        # plot the inliers and outliers in different colors
        ax = fig.add_subplot(111, projection='3d')

        # set limits of all axes
        ax.set_xlim(-10, 150)
        ax.set_ylim(-10, 150)
        ax.set_zlim(-10, 150)

        # draw scatter
        ax.scatter(self._m_features[inliers][:, 0], self._m_features[inliers][:, 1],
                   self._m_features[inliers][:, 2], c='b', marker='o', label='Inlier data', alpha=0.2)
        ax.scatter(self._m_features[outliers][:, 0], self._m_features[outliers][:, 1],
                   self._m_features[outliers][:, 2], c='r', marker='o', label='Outlier data', alpha=0.2)

        # plot the fitted line in green
        line_points = np.linspace(self._m_features[inliers].min(
            axis=0), self._m_features[inliers].max(axis=0), num=len(inliers))
        xyz_line = model_robust.predict(line_points[:, 0])
        ax.plot(xyz_line[:, 0], xyz_line[:, 1],
                xyz_line[:, 2], c='g', label='Fitted line')

        ax.legend(loc='lower left')

    def fitNLines(self, fig,  n, enable_scatter=True, min_samples=2, residual_threshold=3, max_trials=1000):
        ax = fig.add_subplot(111, projection='3d')
        ax.set_xlim(-10, 150)
        ax.set_ylim(-10, 150)
        ax.set_zlim(-10, 150)

        features = self._m_features

        for i in range(n):

            if (len(features) > min_samples):

                model_robust, inliers = ransac(features, LineModelND, min_samples=min_samples,
                                               residual_threshold=residual_threshold, max_trials=max_trials, random_state=0)
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

                # Update features to outliers for the next iteration
                features = features[outliers]
            else:
                print("Couldn't fit any more lines at i = ", i)

        ax.legend(loc='lower left')
