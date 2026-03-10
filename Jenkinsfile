pipeline {
    agent any 

    stages {
        stage('SCANOSS Analysis') {
            agent {
                docker {
                    
                    image 'python:3.9-slim'
                    args '--platform linux/arm64'
                }
            }
            steps {
                script {
                    echo "--- Instalando SCANOSS nativamente en el contenedor ARM ---"
                    
                    sh "pip install scanoss"

                    withCredentials([string(credentialsId: 'scanoss-token', variable: 'TOKEN')]) {
                        echo "--- Executing Scan ---"
                        sh "scanoss-py scan . --apiurl https://api.scanoss.com --key ${TOKEN} --output results.json"
                        
                        echo "--- Converting SARIF ---"
                        sh "scanoss-py convert --input results.json --format sarif --output results.sarif"
                    }

                    // Graphs
                    recordIssues(
                        tools: [sarif(pattern: 'results.sarif', id: 'scanoss', name: 'SCANOSS Analysis')]
                    )
                }
            }
        }
    }
}