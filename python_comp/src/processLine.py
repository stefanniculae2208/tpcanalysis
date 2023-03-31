import pandas as pd
import numpy as np
from skimage.measure import LineModelND, ransac


class processLine:

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
                   self._m_features[inliers][:, 2], c='b', marker='o', label='Inlier data')
        ax.scatter(self._m_features[outliers][:, 0], self._m_features[outliers][:, 1],
                   self._m_features[outliers][:, 2], c='r', marker='o', label='Outlier data')

        # plot the fitted line in green
        line_points = np.linspace(self._m_features[inliers].min(
            axis=0), self._m_features[inliers].max(axis=0), num=len(inliers))
        xyz_line = model_robust.predict(line_points[:, 0])
        ax.plot(xyz_line[:, 0], xyz_line[:, 1],
                xyz_line[:, 2], c='g', label='Fitted line')

        ax.legend(loc='lower left')
