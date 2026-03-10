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
                
                sh """
                    pip3 install --upgrade pip
                    pip3 install scanoss
                """

                withCredentials([string(credentialsId: 'scanoss-token', variable: 'TOKEN')]) {
                    echo "--- Ejecutando Escaneo ---"
                    
                    sh """
                        python3 -m scanoss.cli scan . --apiurl https://api.scanoss.com --key ${TOKEN} --output results.json
                        python3 -m scanoss.cli convert --input results.json --format sarif --output results.sarif
                    """
                }

                echo "--- Graphs ---"
                recordIssues(
                    tools: [sarif(pattern: 'results.sarif', id: 'scanoss', name: 'SCANOSS Analysis')],
                    qualityGates: [[threshold: 1, type: 'TOTAL', criticality: 'UNSTABLE']]
                )
                
                archiveArtifacts artifacts: "*.json, *.sarif", allowEmptyArchive: true
            }
        }
    }
    }
}