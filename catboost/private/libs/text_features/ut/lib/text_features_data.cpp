#include "text_features_data.h"

#include <catboost/private/libs/options/runtime_text_options.h>
#include <catboost/private/libs/text_processing/embedding.h>
#include <catboost/private/libs/text_processing/text_column_builder.h>

#include <util/generic/array_ref.h>
#include <util/generic/vector.h>
#include <util/generic/xrange.h>

using namespace NCB;
using namespace NCBTest;
using namespace NCatboostOptions;

TIntrusivePtr<TMultinomialNaiveBayes> NCBTest::CreateBayes(
    const NCBTest::TTokenizedTextFeature& features,
    TConstArrayRef<ui32> target,
    ui32 numClasses
) {
    auto naiveBayes = MakeIntrusive<TMultinomialNaiveBayes>(CreateGuid(), numClasses);
    TNaiveBayesVisitor bayesVisitor;

    for (ui32 sampleId: xrange(features.size())) {
        bayesVisitor.Update(target[sampleId], features[sampleId], naiveBayes.Get());
    }

    return naiveBayes;
}

TIntrusivePtr<TBM25> NCBTest::CreateBm25(
    const NCBTest::TTokenizedTextFeature& features,
    TConstArrayRef<ui32> target,
    ui32 numClasses
) {
    auto bm25 = MakeIntrusive<TBM25>(CreateGuid(), numClasses);
    TBM25Visitor bm25Visitor;

    for (ui32 sampleId: xrange(features.size())) {
        bm25Visitor.Update(target[sampleId], features[sampleId], bm25.Get());
    }

    return bm25;
}

TIntrusivePtr<TBagOfWordsCalcer> NCBTest::CreateBoW(const TDictionaryPtr& dictionaryPtr) {
    return MakeIntrusive<TBagOfWordsCalcer>(CreateGuid(), dictionaryPtr->Size());
}

static void CreateCalcersAndDependencies(
    TConstArrayRef<TTokenizedTextFeature> tokenizedFeatures,
    TConstArrayRef<TDictionaryPtr> dictionaries,
    TConstArrayRef<ui32> target,
    ui32 textFeatureCount,
    const TRuntimeTextOptions& runtimeTextOptions,
    TVector<TTextFeatureCalcerPtr>* calcers,
    TVector<TVector<ui32>>* perFeatureDictionaries,
    TVector<TVector<ui32>>* perTokenizedFeatureCalcers
) {
    const ui32 classesCount = 2;

    perFeatureDictionaries->resize(textFeatureCount);
    perTokenizedFeatureCalcers->resize(tokenizedFeatures.size());

    for (ui32 tokenizedFeatureIdx : xrange(tokenizedFeatures.size())) {
        const auto& featureDescription = runtimeTextOptions.GetTokenizedFeatureDescription(tokenizedFeatureIdx);

        const auto& dictionary = dictionaries[tokenizedFeatureIdx];
        const ui32 textFeatureIdx = featureDescription.TextFeatureId;
        perFeatureDictionaries->at(textFeatureIdx).push_back(tokenizedFeatureIdx);

        auto& featureCalcers = (*perTokenizedFeatureCalcers)[tokenizedFeatureIdx];
        const auto& tokenizedFeature = tokenizedFeatures[tokenizedFeatureIdx];

        for (const auto& featureCalcer: featureDescription.FeatureEstimators.Get()) {
            const EFeatureCalcerType calcerType = featureCalcer.CalcerType;
            featureCalcers.push_back(calcers->size());

            switch (calcerType) {
                case EFeatureCalcerType::BoW:
                    calcers->push_back(CreateBoW(dictionary));
                    break;
                case EFeatureCalcerType::NaiveBayes:
                    calcers->push_back(CreateBayes(tokenizedFeature, target, classesCount));
                    break;
                case EFeatureCalcerType::BM25:
                    calcers->push_back(CreateBm25(tokenizedFeature, target, classesCount));
                    break;
                default:
                    CB_ENSURE(false, "feature calcer " << calcerType << " is not present in tests");
            }
        }
    }
}

void NCBTest::CreateTextDataForTest(
    TVector<TTextFeature>* features,
    TVector<TTokenizedTextFeature>* tokenizedFeatures,
    TVector<TTextFeatureCalcerPtr>* calcers,
    TVector<TDictionaryPtr>* dictionaries,
    TTokenizerPtr* tokenizer,
    TVector<TVector<ui32>>* perFeatureDictionaries,
    TVector<TVector<ui32>>* perTokenizedFeatureCalcers
) {
    TVector<ui32> target;
    TTextProcessingOptions options;
    TTextDigitizers textDigitizers;
    CreateTextDataForTest(features, tokenizedFeatures, &target, &textDigitizers, &options);

    *tokenizer = textDigitizers.GetTokenizer();
    *dictionaries = textDigitizers.GetDictionaries();

    TRuntimeTextOptions runtimeTextOptions(xrange(features->size()), options);
    CreateCalcersAndDependencies(
        MakeConstArrayRef(*tokenizedFeatures),
        MakeConstArrayRef(*dictionaries),
        MakeConstArrayRef(target),
        features->size(),
        runtimeTextOptions,
        calcers,
        perFeatureDictionaries,
        perTokenizedFeatureCalcers
    );
}

TIntrusivePtr<NCB::TTextProcessingCollection> NCBTest::CreateTextProcessingCollectionForTest() {
    TVector<TTextFeature> features;
    TVector<TTokenizedTextFeature> tokenizedFeatures;
    TVector<TTextFeatureCalcerPtr> calcers;
    TVector<TDictionaryPtr> dictionaries;
    TTokenizerPtr tokenizer;
    TVector<TVector<ui32>> perFeatureDictionaries;
    TVector<TVector<ui32>> perTokenizedFeatureCalcers;
    CreateTextDataForTest(
        &features,
        &tokenizedFeatures,
        &calcers,
        &dictionaries,
        &tokenizer,
        &perFeatureDictionaries,
        &perTokenizedFeatureCalcers
    );
    return MakeIntrusive<TTextProcessingCollection>(
        calcers,
        dictionaries,
        perFeatureDictionaries,
        perTokenizedFeatureCalcers,
        tokenizer
    );
}
