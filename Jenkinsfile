pipeline {
    agent any 

    parameters {
        string(name: 'SCANOSS_API_TOKEN_ID', defaultValue:"scanoss-token", description: 'scanoss-token')
        string(name: 'SCANOSS_CLI_DOCKER_IMAGE', defaultValue:"ghcr.io/scanoss/scanoss-py-jenkins:v1.46.0", description: 'Imagen de SCANOSS')
        string(name: 'SCANOSS_API_URL', defaultValue:"https://api.osskb.org/scan/direct", description: 'https://api.scanoss.com')
    }

    environment {
        
        RESULTS_JSON = "results.json"
        RESULTS_SARIF = "results.sarif"
    }

    stages {
        stage('SCANOSS Scan & Graph') {
            steps {
                script {
                    withCredentials([string(credentialsId: params.SCANOSS_API_TOKEN_ID, variable: 'SCANOSS_API_TOKEN')]) {
                        echo "--- Running container manually ---"
                        
                        
                        sh """
                        docker run --rm --platform linux/arm64 \
                        -v ${WORKSPACE}:/scan \
                        -w /scan \
                        -e SCANOSS_API_TOKEN=${SCANOSS_API_TOKEN} \
                        ${params.SCANOSS_CLI_DOCKER_IMAGE} \
                        /bin/sh -c "scanoss-py scan . --apiurl ${params.SCANOSS_API_URL} --key ${SCANOSS_API_TOKEN} --output ${env.RESULTS_JSON} && \
                                    scanoss-py convert --input ${env.RESULTS_JSON} --format sarif --output ${env.RESULTS_SARIF}"
                        """
                    }

                    echo "--- Generating graphs Jenkins ---"
                    
                    recordIssues(
                        tools: [sarif(pattern: env.RESULTS_SARIF, id: 'scanoss', name: 'SCANOSS SBOM Analysis')],
                        qualityGates: [[threshold: 1, type: 'TOTAL', criticality: 'UNSTABLE']]
                    )
                    
                    archiveArtifacts artifacts: "*.json, *.sarif", allowEmptyArchive: true
                }
            }
        }
    }
}