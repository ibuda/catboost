#include "approx_calcer_helpers.h"

#include <catboost/private/libs/algo_helpers/approx_calcer_helpers.h>

void CreateBacktrackingObjective(
    const TLearnContext& ctx,
    bool* haveBacktrackingObjective,
    double* minimizationSign,
    TVector<THolder<IMetric>>* lossFunction) {
    const auto& treeOptions = ctx.Params.ObliviousTreeOptions.Get();
    CreateBacktrackingObjective(
        ctx.LearnProgress->ApproxDimension,
        treeOptions.LeavesEstimationIterations.Get(),
        treeOptions.LeavesEstimationBacktrackingType,
        ctx.Params.MetricOptions->ObjectiveMetric,
        haveBacktrackingObjective,
        minimizationSign,
        lossFunction);
}
