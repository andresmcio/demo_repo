pipeline {
    agent any 

    stages {
        stage('SCANOSS Analysis') {
            agent {
                docker {
                    image 'python:3.9-slim'
                    args '-u root --platform linux/arm64'
                }
            }
            steps {
                script {
                    echo "--- Installing SCANOSS ---"
                    sh "pip3 install scanoss"

                    withCredentials([string(credentialsId: 'scanoss-token', variable: 'TOKEN')]) {
                    echo "--- Scanning ---"
                    sh "python3 -m scanoss.cli scan . --apiurl https://api.scanoss.com --key ${TOKEN} --output results.json"
                    
                    echo "--- Converting toCycloneDX ---"
                    sh "python3 -m scanoss.cli convert --input results.json --format cyclonedx --output results.json"
                }

                echo "--- Results ---"
                
                recordIssues(
                    enabledForFailure: true,
                    aggregatingResults: true,
                    tool: issues(pattern: 'results.json', id: 'scanoss', name: 'SCANOSS Analysis'),
                    qualityGates: [[threshold: 1, type: 'TOTAL', criticality: 'UNSTABLE']]
                )
                    
                    archiveArtifacts artifacts: "results.json, results.xml", allowEmptyArchive: true
                }
            }
        }
    }
}