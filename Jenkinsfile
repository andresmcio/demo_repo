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
                        
                        
                        sh "scanoss-py scan . --apiurl ${params.SCANOSS_API_URL} --key ${SCANOSS_API_TOKEN} --output results.json"
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