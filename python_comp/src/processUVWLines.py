import pandas as pd
import numpy as np
from skimage.measure import LineModelND, ransac
import matplotlib.pyplot as plt


class processUVWLines:
    def __init__(self, entry):
        self._m_entry = entry

        entry_u = entry[entry['plane'] == 0]
        entry_v = entry[entry['plane'] == 1]
        entry_w = entry[entry['plane'] == 2]

        self._m_hits_u = entry_u[['time_bin', 'strip']].to_numpy()
        self._m_hits_v = entry_v[['time_bin', 'strip']].to_numpy()
        self._m_hits_w = entry_w[['time_bin', 'strip']].to_numpy()

    def setEntry(self, entry):
        self._m_entry = entry

        entry_u = entry[entry['plane'] == 0]
        entry_v = entry[entry['plane'] == 1]
        entry_w = entry[entry['plane'] == 2]

        self._m_hits_u = entry_u[['time_bin', 'strip']].to_numpy()
        self._m_hits_v = entry_v[['time_bin', 'strip']].to_numpy()
        self._m_hits_w = entry_w[['time_bin', 'strip']].to_numpy()

    def drawHits(self, fig, axes):
        # Plot the data for plane 0
        axes[0].scatter(self._m_hits_u[:, 0], self._m_hits_u[:, 1])
        axes[0].set_title('Plane U')
        axes[0].set_xlim(-10, 520)
        axes[0].set_ylim(-10, 100)

        # Plot the data for plane 1
        axes[1].scatter(self._m_hits_v[:, 0], self._m_hits_v[:, 1])
        axes[1].set_title('Plane V')
        axes[1].set_xlim(-10, 520)
        axes[1].set_ylim(-10, 100)

        # Plot the data for plane 2
        axes[2].scatter(self._m_hits_w[:, 0], self._m_hits_w[:, 1])
        axes[2].set_title('Plane W')
        axes[2].set_xlim(-10, 520)
        axes[2].set_ylim(-10, 100)

    def fitNLines(self, fig, axes,  n, enable_scatter=True, min_samples=2, residual_threshold=3, max_trials=1000):

        axes[0].set_title('Plane U')
        axes[0].set_xlim(-10, 520)
        axes[0].set_ylim(-10, 100)

        axes[1].set_title('Plane V')
        axes[1].set_xlim(-10, 520)
        axes[1].set_ylim(-10, 100)

        axes[2].set_title('Plane W')
        axes[2].set_xlim(-10, 520)
        axes[2].set_ylim(-10, 100)

        # Start with plane U
        features = self._m_hits_u

        for i in range(n):

            if (len(features) > min_samples):

                model_robust_u, inliers = ransac(features, LineModelND, min_samples=min_samples,
                                                 residual_threshold=residual_threshold, max_trials=max_trials, random_state=0)
                outliers = inliers == False

                if (enable_scatter == True):
                    axes[0].scatter(features[inliers][:, 0], features[inliers][:, 1],
                                    c='b', marker='o', label=f'Inliers {i+1}', alpha=0.2)
                    axes[0].scatter(features[outliers][:, 0], features[outliers][:, 1],
                                    c='r', marker='o', label=f'Outliers {i+1}', alpha=0.2)

                line_points = np.linspace(features[inliers].min(
                    axis=0), features[inliers].max(axis=0), num=len(inliers))
                xy_line = model_robust_u.predict(line_points[:, 0])
                axes[0].plot(xy_line[:, 0], xy_line[:, 1],
                             label=f'Fitted line {i+1}')

                # Update features to outliers for the next iteration
                features = features[outliers]
            else:
                print("Couldn't fit any more lines for plane U at i = ", i)

        axes[0].legend(loc='lower left')

        # Then plane V
        features = self._m_hits_v

        for i in range(n):

            if (len(features) > min_samples):

                model_robust_v, inliers = ransac(features, LineModelND, min_samples=min_samples,
                                                 residual_threshold=residual_threshold, max_trials=max_trials, random_state=0)
                outliers = inliers == False

                if (enable_scatter == True):
                    axes[1].scatter(features[inliers][:, 0], features[inliers][:, 1],
                                    c='b', marker='o', label=f'Inliers {i+1}', alpha=0.2)
                    axes[1].scatter(features[outliers][:, 0], features[outliers][:, 1],
                                    c='r', marker='o', label=f'Outliers {i+1}', alpha=0.2)

                line_points = np.linspace(features[inliers].min(
                    axis=0), features[inliers].max(axis=0), num=len(inliers))
                xy_line = model_robust_v.predict(line_points[:, 0])
                axes[1].plot(xy_line[:, 0], xy_line[:, 1],
                             label=f'Fitted line {i+1}')

                # Update features to outliers for the next iteration
                features = features[outliers]
            else:
                print("Couldn't fit any more lines for plane V at i = ", i)

        axes[1].legend(loc='lower left')

        # Finally plane W
        features = self._m_hits_w

        for i in range(n):

            if (len(features) > min_samples):

                model_robust_w, inliers = ransac(features, LineModelND, min_samples=min_samples,
                                                 residual_threshold=residual_threshold, max_trials=max_trials, random_state=0)
                outliers = inliers == False

                if (enable_scatter == True):
                    axes[2].scatter(features[inliers][:, 0], features[inliers][:, 1],
                                    c='b', marker='o', label=f'Inliers {i+1}', alpha=0.2)
                    axes[2].scatter(features[outliers][:, 0], features[outliers][:, 1],
                                    c='r', marker='o', label=f'Outliers {i+1}', alpha=0.2)

                line_points = np.linspace(features[inliers].min(
                    axis=0), features[inliers].max(axis=0), num=len(inliers))
                xy_line = model_robust_w.predict(line_points[:, 0])
                axes[2].plot(xy_line[:, 0], xy_line[:, 1],
                             label=f'Fitted line {i+1}')

                # Update features to outliers for the next iteration
                features = features[outliers]
            else:
                print("Couldn't fit any more lines for plane W at i = ", i)

        axes[2].legend(loc='lower left')

        return [model_robust_u, model_robust_v, model_robust_w]
